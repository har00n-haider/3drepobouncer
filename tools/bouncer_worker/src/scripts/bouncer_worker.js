#!/usr/bin/env node

/**
 *	Copyright (C) 2015 3D Repo Ltd
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU Affero General Public License as
 *	published by the Free Software Foundation, either version 3 of the
 *	License, or (at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Affero General Public License for more details.
 *
 *	You should have received a copy of the GNU Affero General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>
 * /

 /*
  * Bouncer Worker - The RabbitMQ Client for bouncer library
  * To run this:
  *  - ensure NodeJS/npm is installed
  *  - create config.js (use config_example.js as your base)
  *  - npm install amqplib
  *  - node bouncer_worker.js
  */

/*jshint esversion: 6 */

(() => {
	"use strict";
	const path = require("path");
	const run = require("../lib/runCommand");


	const amqp = require("amqplib");
	const importToy = require('../toy/importToy');
	const rootModelDir = './toy';
	//Note: these error codes corresponds to error_codes.h in bouncerclient
	const ERRCODE_BOUNCER_CRASH = 12;
	const ERRCODE_PARAM_READ_FAIL = 13;
	const ERRCODE_BUNDLE_GEN_FAIL = 14;
	const ERRCODE_TIMEOUT = 29;
	const softFails = [7,10,15]; //failures that should go through to generate bundle
	let retry = 0;
	let connClosed = false;

	const { config, configPath}  = require("../lib/config");

	const logger = require("../lib/logger");

	/**
	 * Test that the client is working and
	 * it is able to connect to the database
	 * Takes a callback function to with
	 * indication of success or failure
	 */
	function testClient(callback){
		logger.info("Checking status of client...");

		setBouncerEnvars();

		let awsBucketName = "undefined";
		let awsBucketRegion = "undefined";

		if (config.aws)
		{
			awsBucketName = config.aws.bucket_name;
			awsBucketRegion = config.aws.bucket_region;
		}

		const cmdParams = [
				configPath,
				"test"
			];


		run(path.normalize(config.bouncer.path), cmdParams).then(() => {
			logger.info("Bouncer call passed");
			callback();
		}).catch((code)=> {
			logger.error(`Bouncer call errored (Error code: ${code})`);
		});
	}

	/**
	 * handle queue message
	 */
	function handleMessage(cmd, rid, callback){
		// command start with importToy is handled here instead of passing it to bouncer
		if(cmd.startsWith('importToy')){

			let args = cmd.split(' ');

			let database = args[1];
			let model = args[2];
			let modelDir = args[3];
			let skipPostProcessing = args[4];
			skipPostProcessing = skipPostProcessing && JSON.parse(skipPostProcessing) || {};

			let username = database;

			let dbConfig = {
				username: config.db.username,
				password: config.db.password,
				dbhost: config.db.dbhost,
				dbport: config.db.dbport,
				writeConcern: config.mongoimport && config.mongoimport.writeConcern
			};

			let dir = `${rootModelDir}/${modelDir}`;

			importToy(dbConfig, dir, username, database, model, skipPostProcessing).then(() => {
				// after importing the toy regenerate the tree as well
				if(skipPostProcessing.tree){
					callback(JSON.stringify({
						value: 0,
						database: database,
						project: model
					}), true);
				} else {
					exeCommand(`genStash ${database} ${model} tree all`, rid, callback);
				}

			}).catch(err => {

				logger.error("importToy module error");

				callback(JSON.stringify({
					value: ERRCODE_BOUNCER_CRASH,
					message: err.message,
					database: database,
					project: model
				}), true);
			});

		} else {
			exeCommand(cmd, rid, callback);
		}

	}

	function setBouncerEnvars(logDir) {
		if (config.aws)
		{
			process.env['AWS_ACCESS_KEY_ID'] = config.aws.access_key_id;
			process.env['AWS_SECRET_ACCESS_KEY'] =	config.aws.secret_access_key;
		}

		if (config.bouncer.envars) {
			Object.keys(config.bouncer.envars).forEach((key) => {
				process.env[key] = config.bouncer.envars[key];
			});
		}

		if(logDir) {
			process.env['REPO_LOG_DIR']= logDir ;
		}
	}

	function runBouncer(logDir, cmd,  callback)
	{
		const os = require('os');
		let command = "";
		const cmdParams = [];

		let awsBucketName = "undefined";
		let awsBucketRegion = "undfined";

		setBouncerEnvars(logDir);

		if (config.aws)
		{
			awsBucketName = config.aws.bucket_name;
			awsBucketRegion = config.aws.bucket_region;
		}

		cmdParams.push(configFullPath);

		if(os.platform() === "win32")
		{

			cmd = cmd.replace("/sharedData/", config.rabbitmq.sharedDir);

			command = path.normalize(config.bouncer.path);
			cmd.split(' ').forEach((data) => cmdParams.push(data));

			let cmdArr = cmd.split(' ');
			if(cmdArr[0] == "import")
			{
				const fs = require('fs')
				fs.readFile(cmdArr[2], 'utf8', function (err,data) {
					if (err) {
						return logger.error(err);
					}
					let result = data.replace("/sharedData/", config.rabbitmq.sharedDir);

					fs.writeFile(cmdArr[2], result, 'utf8', function (err) {
							if (err) return logger.error(err);
					});
				});
			}
		}
		else
		{
			cmd.split(' ').forEach((data) => cmdParams.push(data));
		}

		let cmdFile;
		let cmdDatabase;
		let cmdProject;
		let cmdArr = cmd.split(' ');
		let toyFed = false;
		let user = "";

		// Extract database and project information from command
		try{
			switch(cmdArr[0]) {
				case "import":
					cmdFile = require(cmdArr[2]);
					cmdDatabase = cmdFile.database;
					cmdProject = cmdFile.project;
					user = cmdFile.owner;
					break;
				case "genFed":
					cmdFile = require(cmdArr[1]);
					cmdDatabase = cmdFile.database;
					cmdProject = cmdFile.project;
					toyFed = cmdFile.toyFed;
					user = cmdFile.owner;
					break;
				case "importToy":
					cmdDatabase = cmdArr[1];
					cmdProject = cmdArr[2];
					break;
				case "genStash":
					cmdDatabase = cmdArr[1];
					cmdProject = cmdArr[2];
					break;
				default:
					logger.error("Unexpected command: " + cmdArr[0]);
			}
		}
		catch(error) {
			logger.error(error);
			callback({value: 16}, true);
		}

		// Issue callback to indicate job is processing, but no ack as job not done
		if ("genFed" !== cmdArr[0]) {
			callback({
				status: "processing",
				database: cmdDatabase,
				project: cmdProject
			}, false);
		}

		const execProm = run(path.normalize(config.bouncer.path), cmdParams, softFails);

		execProm.then((code) => {
			logger.info(`[SUCCEED] Executed command: ${command} ${cmdParams.join(" ")} `, code);
			if(config.unity && config.unity.project && cmdArr[0] == "import")
			{
				let commandArgs = cmdFile;
				if(commandArgs && commandArgs.database && commandArgs.project)
				{
					let awsBucketName = "undefined";
					let awsBucketRegion = "undefined";

					if (config.aws)
					{
						process.env['AWS_ACCESS_KEY_ID'] = config.aws.access_key_id;
						process.env['AWS_SECRET_ACCESS_KEY'] =	config.aws.secret_access_key;
						awsBucketName = config.aws.bucket_name;
						awsBucketRegion = config.aws.bucket_region;
					}

					const unityCommand = config.unity.batPath;
					const unityCmdParams = [
							config.unity.project,
							configFullPath,
							commandArgs.database,
							commandArgs.project,
							logDir
					];

					logger.info(`Running unity command: ${unityCommand} ${unityCmdParams.join(" ")}`);
					const unityExec = run(unityCommand, unityCmdParams);
					unityExec.then((unityCode) => {
						logger.info(`[SUCCESS] Executed unity command: ${unityCommand} ${unityCmdParams.join(" ")}`, unityCode);
						callback({
							value: code,
							database: cmdDatabase,
							project: cmdProject,
							user
						}, true);
					}).catch((unityCode) => {
						logger.info(`[FAILED] Executed unity command: ${unityCommand} ${unityCmdParams.join(" ")}`, unityCode);
						callback({
							value: ERRCODE_BUNDLE_GEN_FAIL,
							database: cmdDatabase,
							project: cmdProject,
							user
						}, true);
					});
				}
				else
				{
					logger.error("Failed to read " + cmdArr[2]);
					callback({
						value: ERRCODE_PARAM_READ_FAIL,
						database: cmdDatabase,
						project: cmdProject,
						user
					}, true);
				}
			}
			else
			{
				if(toyFed) {

					const dbConfig = {
						username: config.db.username,
						password: config.db.password,
						dbhost: config.db.dbhost,
						dbport: config.db.dbport,
						writeConcern: config.mongoimport && config.mongoimport.writeConcern
					};
					const dir = `${rootModelDir}/${toyFed}`;
					importToy(dbConfig, dir, cmdDatabase, cmdDatabase, cmdProject, {tree: 1}).then(()=> {
						callback({
							value: code,
							database: cmdDatabase,
							project: cmdProject,
							user
						}, true);
					});
				} else {

					callback({
						value: code,
						database: cmdDatabase,
						project: cmdProject,
						user
					}, true);
				}
			}

		}).catch((code) => {
			const err =  code || ERRCODE_BOUNCER_CRASH;
			callback({
				value: err,
				database: cmdDatabase,
				project: cmdProject,
				user
			}, true);
			logger.error(`[FAILED] Executed command: ${command} ${cmdParams.join(" ")}`, err);
		});

	}

	/**
	 * Execute the Command and provide a reply message to the callback function
	 */
	function exeCommand(cmd, rid, callback){

		let logRootDir = config.bouncer.log_dir;

		if(logRootDir === null) {
			logRootDir = "./log";
		}

		let logDir = logRootDir + "/" +  rid.toString() + "/";

		runBouncer(logDir, cmd,
		 function(reply, sendAck){
			callback(JSON.stringify(reply), sendAck);
		});
	}

	/*
	 * @param {sendAck} sendAck - Should an acknowledgement be sent with callback (true/false)
	 */
	function listenToQueue(ch, queueName, prefetchCount)
	{
		ch.assertQueue(queueName, {durable: true});
		logger.info("Bouncer Client Queue started. Waiting for messages in %s of %s....", queueName, config.rabbitmq.host);
		ch.prefetch(prefetchCount);
		ch.consume(queueName, function(msg){
			logger.info(" [x] Received %s from %s", msg.content.toString(), queueName);
			handleMessage(msg.content.toString(), msg.properties.correlationId, function(reply, sendAck){
				if (sendAck)
					ch.ack(msg);
				logger.info("sending to reply queue(%s): %s", config.rabbitmq.callback_queue, reply);
				ch.sendToQueue(config.rabbitmq.callback_queue, new Buffer.from(reply),
					{correlationId: msg.properties.correlationId, appId: msg.properties.appId});
			});
		}, {noAck: false});
	}

	function reconnectQ() {
		const maxRetries = config.rabbitmq.maxRetries || 3;
		if(++retry <= maxRetries) {
			logger.error(`[AMQP] Trying to reconnect [${retry}/${maxRetries}]...`);
			connectQ();
		} else {
			logger.error("[AMQP] Retries exhausted");
			process.exit(-1);
		}
	}

	function connectQ(){
		amqp.connect(config.rabbitmq.host).then((conn) => {
			retry = 0;
			connClosed = false;
			logger.info("[AMQP] Connected! Creating channel...");
			conn.createChannel().then((ch) => {
				ch.assertQueue(config.rabbitmq.callback_queue, { durable: true });
				config.rabbitmq.worker_queue && listenToQueue(ch, config.rabbitmq.worker_queue, config.rabbitmq.task_prefetch || 4);
				config.rabbitmq.model_queue && listenToQueue(ch, config.rabbitmq.model_queue, config.rabbitmq.model_prefetch || 1);

			});

			conn.on("close", () => {
				if(!connClosed) {
					//this can be called more than once for some reason. Use a boolean to distinguish first timers.
					connClosed = true;
					logger.error("[AMQP] connection closed.");
					reconnectQ();
				}
			});

			conn.on("error", (err)	=> {
				logger.error("[AMQP] connection error: " + err.message);
			});


		}).catch((err) => {
			logger.error(`[AMQP] failed to establish connection to rabbit mq: ${err}.`);
			reconnectQ();
		});
	}

	logger.info("Initialising bouncer client queue...");
	if(config.hasOwnProperty("umask")) {
		logger.info("Setting umask: " + config.umask);
		process.umask(config.umask);
	}
	testClient(connectQ);

})();


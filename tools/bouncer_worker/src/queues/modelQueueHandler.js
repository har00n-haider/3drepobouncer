/**
 * Copyright (C) 2020 3D Repo Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

const fs = require('fs');
const {
	callbackQueueSpecified,
	modelQueueSpecified,
	unityQueueSpecified,
	logDirExists,
	sharedDirExists } = require('./common');
const { config } = require('../lib/config');
const { runBouncerCommand } = require('../tasks/bouncerClient');
const { ERRCODE_OK, ERRCODE_BOUNCER_CRASH } = require('../constants/errorCodes');
const { MODEL_PROCESSING, UNITY_QUEUED } = require('../constants/statuses');
const { messageDecoder } = require('../lib/messageDecoder');
const logger = require('../lib/logger');
const Utils = require('../lib/utils');

const Handler = {};
const logLabel = { label: 'MODELQ' };

Handler.onMessageReceived = async (cmd, rid, callback) => {
	const logDir = `${config.logging.taskLogDir}/${rid.toString()}/`;
	const { errorCode, database, model, user, cmdParams, file } = messageDecoder(cmd);

	if (errorCode) {
		callback(JSON.stringify({ value: errorCode }));
		return;
	}

	callback(JSON.stringify({
		status: MODEL_PROCESSING,
		database,
		project: model,
	}));

	const returnMessage = {
		value: ERRCODE_OK,
		database,
		project: model,
		user,
	};

	try {
		const { size } = fs.statSync(file);
		const processInformation = Utils.gatherProcessInformation(
			user,
			model,
			database,
			logLabel.label, // queue
			file.split('.').pop().toString(), // filetype
			size, // filesize
		);

		returnMessage.value = await runBouncerCommand(logDir, cmdParams, processInformation);

		callback(JSON.stringify(returnMessage), config.rabbitmq.unity_queue);
		callback(JSON.stringify({
			status: UNITY_QUEUED,
			database,
			project: model,
		}));
	} catch (err) {
		logger.error(`Import model error: ${err.message || err}`, logLabel);
		returnMessage.value = err || ERRCODE_BOUNCER_CRASH;
		callback(JSON.stringify(returnMessage));
	}
};

Handler.validateConfiguration = (label) => modelQueueSpecified(label)
		&& callbackQueueSpecified(label)
		&& unityQueueSpecified(label)
		&& logDirExists(label)
		&& sharedDirExists(label);

module.exports = Handler;

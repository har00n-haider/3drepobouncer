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

const moment = require('moment');

const Utils = {};

const CryptoJS = require('crypto-js');

Utils.hashCode = (s) => CryptoJS.MD5(s).toString();

Utils.exitApplication = (errCode = -1) => {
	// eslint-disable-next-line no-process-exit
	process.exit(errCode);
};

Utils.sleep = (ms) => new Promise((resolve) => {
	setTimeout(resolve, ms);
});

Utils.getCurrentDateTimeAsString = () => moment().format('YYYY-MM-DD_HH[h]mm[m]ss[s]');

Utils.gatherProcessInformation = (
	owner,
	model,
	database,
	queue,
	filetype,
	filesize,
) => (
	{
		DateTime: Date.now(),
		Owner: owner,
		Model: model,
		Database: database,
		Queue: queue, // logLabel.label,
		FileType: filetype, // file.split('.').pop().toString(),
		FileSize: filesize, // fileStats.size,
	});

module.exports = Utils;

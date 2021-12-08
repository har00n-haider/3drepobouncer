/**
*  Copyright (C) 2021 3D Repo Ltd
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU Affero General Public License as
*  published by the Free Software Foundation, either version 3 of the
*  License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Affero General Public License for more details.
*
*  You should have received a copy of the GNU Affero General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "cryptolens/core.hpp"
#include "cryptolens/Error.hpp"
#include "cryptolens/Configuration_Windows.hpp"
#include "cryptolens/MachineCodeComputer_static.hpp"
#include "repo_log.h"
#include "datastructure/repo_uuid.h"
#include "repo_exception.h"


namespace cryptolens = ::cryptolens_io::v20190401;

using Cryptolens = cryptolens::basic_Cryptolens<cryptolens::Configuration_Windows<cryptolens::MachineCodeComputer_static>>;

namespace Licensing
{

	static const std::string licenseEnvVarName = "REPO_LICENSE";
	static const std::string pubKeyModulus = LICENSE_RSA_PUB_KEY_MOD;
	static const std::string pubKeyExponent = LICENSE_RSA_PUB_KEY_EXP;
	static const std::string authToken = LICENSE_AUTH_TOKEN;
	static const int floatingTimeIntervalSec = LICENSE_TIMEOUT_SECONDS;
	static const int productId = LICENSE_PRODUCT_ID;

	class LicenseValidator
	{
	private:
		static std::string licenseStr;
		static repo::lib::RepoUUID instanceId;
		static Cryptolens cryptolens_handle;

	public:

		/**
		* Uses the cryptolens API to verify a floating license
		* 
		* TODO: write description - Exception when license is not valid/expired
		*/
		static void RunActivation()
		{
#ifdef REPO_LICENSE_CHECK
			// only run once 
			if (!instanceId.isDefaultValue()) return;

			// get the license string
			licenseStr = GetLicenseString();

			// attempt activation
			cryptolens::Error e;
			Cryptolens cryptolens_handle(e);

			// seting the public key
			cryptolens_handle.signature_verifier.set_modulus_base64(e, pubKeyModulus);
			cryptolens_handle.signature_verifier.set_exponent_base64(e, pubKeyExponent);

			// machine/instance id
			// Using machine code to store the instance UUID instead, as per
			// https://help.cryptolens.io/licensing-models/containers
			// TODO: need to get a UUID for the instance session
			instanceId = repo::lib::RepoUUID::createUUID();
			cryptolens_handle.machine_code_computer.set_machine_code(e, instanceId.toString());

			// License activation 
			// License key will not be saved, and we activation will be done each time,
			// as we don't want offline verification periods.
			cryptolens::optional<cryptolens::LicenseKey> license_key =
				cryptolens_handle.activate_floating
				(
					// Object used for reporting if an error occured
					e,
					// Cryptolens Access Token
					authToken,
					// Product key associated with the 3drepo.io pipeline
					productId,
					// License Key
					licenseStr,
					// The amount of time the user has to wait before an actived
					// machine (or session in our case) is taken off the license.
					floatingTimeIntervalSec,
					10
				);

			// dealing with the result
			repoInfo << "****License activation summary****";
			if (e)
			{
				cryptolens::ActivateError error = cryptolens::ActivateError::from_reason(e.get_reason());
				repoInfo << "- server error: " << error.what();
				repoInfo << "- license check: false";
				//TODO: make proper exception class for this
				repoInfo << "- session not added to license ";
				throw repo::lib::RepoValidityExpiredException();
			}

			repoInfo << "- session license ID: " << instanceId.toString();
			//repoInfo << "- server message: " << license_key->get_notes() ? *license_key->get_notes() : "";
			repoInfo << "- license check: true";
			//repoInfo << "- instance usage: " << 
			//	(license_key->get_activated_machines() ? license_key->get_activated_machines()->size() : 0)<<
			//	"/" << 
			//	(license_key->get_maxnoofmachines() ? *license_key->get_maxnoofmachines() : 0);
			repoInfo << "- session succesfully added to license";


#endif
		}


		/**
		* TODO: figure out how to do this with th C++ API
		*/
		static void RunDeactivation()
		{
#ifdef REPO_LICENSE_CHECK
			// only deactivate if we have succesfully activated
			if (instanceId.isDefaultValue()) return;

			// attempt deactivation here
			//repoInfo << "****License deactivation summary****\n";
			//repoInfo << "- license message: {result.Message}\n";
			//repoInfo << "- session license ID: {deactivateModel.MachineCode}\n";
			//if (true)
			//{
			//	repoInfo <<
			//		"- deactivation message: Error trying to deactivate license, " <<
			//		"this instance will be taken off the license in less than" <<
			//		"{LicenseConfig.floatingLicenseTimeout} seconds\n";
			//}
			//else
			//{
			//	repoInfo <<"- session succesfully removed from license";
			//}
#endif
		}

	private:

		static std::string GetLicenseString()
		{
			std::string licenseStr;
			char * licenseStrPtr = getenv(licenseEnvVarName.c_str());
			if(licenseStrPtr && strlen(licenseStrPtr) > 0)
			{
				licenseStr = std::string(licenseStrPtr);
			}
			else 
			{
				repoError <<
					"License not found, expected to find it in this " <<
					"environment variable: " <<
					licenseEnvVarName;
				throw repo::lib::RepoValidityExpiredException();
			}
			return licenseStr;
		}

	};

}
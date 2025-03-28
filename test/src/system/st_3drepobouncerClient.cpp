/**
*  Copyright (C) 2015 3D Repo Ltd
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

/**
* This tests that 3DRepobouncerClient performs as expected
* In terms of returning the right signals back to the caller
*/

#include <cstdlib>
#include <stdlib.h>
#include <gtest/gtest.h>
#include <repo/error_codes.h>
#include <repo/repo_controller.h>
#include "../unit/repo_test_database_info.h"
#include "../unit/repo_test_utils.h"

static std::string getSuccessFilePath()
{
	return getDataPath(simpleModel);
}

static std::string produceCleanArgs(
	const std::string &database,
	const std::string &project,
	const std::string &dbAdd = REPO_GTEST_DBADDRESS,
	const int         &port = REPO_GTEST_DBPORT,
	const std::string &username = REPO_GTEST_DBUSER,
	const std::string &password = REPO_GTEST_DBPW
)
{
	return  getClientExePath() + " "
		+ getConnConfig()
		+ " clean "
		+ database + " "
		+ project;
}

static std::string produceGenStashArgs(
	const std::string &database,
	const std::string &project,
	const std::string &type
)
{
	return  getClientExePath() + " "
		+ getConnConfig()
		+ " genStash "
		+ database + " "
		+ project + " "
		+ type;
}

static std::string produceGetFileArgs(
	const std::string &file,
	const std::string &database,
	const std::string &project
)
{
	return  getClientExePath() + " "
		+ getConnConfig()
		+ " getFile "
		+ database + " "
		+ project + " \""
		+ file + "\"";
}

static std::string produceCreateFedArgs(
	const std::string &file,
	const std::string &owner = std::string()
)
{
	return  getClientExePath() + " "
		+ getConnConfig()
		+ " genFed \""
		+ file + "\" "
		+ owner;
}

static std::string produceUploadFileArgs(
	const std::string &filePath
) {
	return  getClientExePath() + " "
		+ getConnConfig()
		+ " import -f \""
		+ filePath + "\"";
}

static std::string produceUploadArgs(
	const std::string &database,
	const std::string &project,
	const std::string &filePath,
	const std::string &configPath = getConnConfig())
{
	return  getClientExePath()
		+ " " + configPath
		+ " import \""
		+ filePath + "\" "
		+ database + " " + project;
}

static int runProcess(
	const std::string &cmd)
{
	int status = system(cmd.c_str());
#ifndef _WIN32
	//Linux, use WIFEXITED(status) to get the real exit code
	return WEXITSTATUS(status);
#else
	return status;
#endif
}

static int testUpload(
	std::string mongoDbName,
	std::string projectName,
	std::string fileName
)
{
	std::string uploadCmd = produceUploadArgs(
		mongoDbName,
		projectName,
		getDataPath(fileName));

	int errCode = runProcess(uploadCmd);
	;
	repoInfo << "Error code from bouncer client: " << errCode
		<< ", " << (int)errCode;
	return errCode;
};

TEST(RepoClientTest, UploadTestInvalidDBConn)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));

	//Test failing to connect to database
	std::string db = "stUpload";
	std::string failToConnect = produceUploadArgs(db, "failConn", getSuccessFilePath(), getDataPath("config/invalidAdd.json"));
	EXPECT_EQ((int)REPOERR_AUTH_FAILED, runProcess(failToConnect));
	EXPECT_FALSE(projectExists(db, "failConn"));
}

TEST(RepoClientTest, UploadTestBadDBAuth)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	//Test Bad authentication
	std::string failToAuth = produceUploadArgs(db, "failAuth", getSuccessFilePath(), getDataPath("config/badAuth.json"));
	EXPECT_EQ((int)REPOERR_AUTH_FAILED, runProcess(failToAuth));
	EXPECT_FALSE(projectExists(db, "failAuth"));
}

TEST(RepoClientTest, UploadTestNoFile)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";
	//Test Bad FilePath
	std::string badFilePath = produceUploadArgs(db, "failPath", "nonExistentFile.obj");
	fflush(stdout);
	std::cout << " Executing: " << badFilePath << std::endl;
	fflush(stdout);
	EXPECT_EQ((int)REPOERR_MODEL_FILE_READ, runProcess(badFilePath));
	EXPECT_FALSE(projectExists(db, "failPath"));
}

TEST(RepoClientTest, UploadTestBadExt)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	//Test Bad extension
	std::string badExt = produceUploadArgs(db, "failExt", getDataPath(badExtensionFile));
	EXPECT_EQ((int)REPOERR_FILE_TYPE_NOT_SUPPORTED, runProcess(badExt));
	EXPECT_FALSE(projectExists(db, "failExt"));
}

TEST(RepoClientTest, UploadTestBadFBXVer)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	//Unsupported FBX version
	std::string unsupportedVersion = produceUploadArgs(db, "failExt", getDataPath(unsupportedFBXVersion));
	EXPECT_EQ((int)REPOERR_UNSUPPORTED_FBX_VERSION, runProcess(unsupportedVersion));
	EXPECT_FALSE(projectExists(db, "failExt"));
}

TEST(RepoClientTest, UploadTestBadArgs)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));

	//Insufficient arguments
	std::string lackArg = getClientExePath() + " " + getConnConfig() + " import " + getSuccessFilePath();
	EXPECT_EQ((int)REPOERR_INVALID_ARG, runProcess(lackArg));
}

TEST(RepoClientTest, UploadTestSuccess)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	//Test Good Upload
	std::string goodUpload = produceUploadArgs(db, "cube", getSuccessFilePath());
	EXPECT_EQ((int)REPOERR_OK, runProcess(goodUpload));
	EXPECT_TRUE(projectExists(db, "cube"));

	EXPECT_EQ((int)REPOERR_OK, runProcess(produceUploadFileArgs(getDataPath(importSuccess))));
	EXPECT_TRUE(projectExists("testDB", importSuccessPro));
	EXPECT_TRUE(projectSettingsCheck("testDB", importSuccessPro, "owner", "", ""));

	EXPECT_EQ((int)REPOERR_OK, runProcess(produceUploadFileArgs(getDataPath(importSuccess2))));
	EXPECT_TRUE(projectExists("testDB", importSuccessPro2));
	EXPECT_TRUE(projectSettingsCheck("testDB", importSuccessPro2, "owner", "taggg", "desccc"));
}

TEST(RepoClientTest, UploadTestTexture)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	//Test Textured Upload
	std::string texUpload = produceUploadArgs(db, "textured", getDataPath(texturedModel));
	EXPECT_EQ((int)REPOERR_LOAD_SCENE_MISSING_TEXTURE, runProcess(texUpload));
	EXPECT_TRUE(projectExists(db, "textured"));
}

TEST(RepoClientTest, UploadTestMissingNodes)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	//Test missing nodes Upload
	std::string misUpload = produceUploadArgs(db, "missing", getDataPath(missingNodesModel));
	std::cout << " Running Missing nodes... " << misUpload << std::endl;
	EXPECT_EQ((int)REPOERR_LOAD_SCENE_MISSING_NODES, runProcess(misUpload));
	EXPECT_TRUE(projectExists(db, "missing"));
}

TEST(RepoClientTest, UploadTestBIM)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));

	std::string mongoDbName = "stUpload";

	std::string spoofedBim1PrjName = "spoofedBIM1Test";
	EXPECT_EQ(REPOERR_UNSUPPORTED_BIM_VERSION,
		testUpload(mongoDbName, spoofedBim1PrjName, "RepoModelImport/cube_bim1_spoofed.bim"));
	EXPECT_FALSE(projectExists(mongoDbName, spoofedBim1PrjName));

	std::string okBim2PrjName = "okBIM2Test";
	EXPECT_EQ(REPOERR_OK,
		testUpload(mongoDbName, okBim2PrjName, "RepoModelImport/cube_bim2_navis_2021_repo_4.6.1.bim"));
	EXPECT_TRUE(projectExists(mongoDbName, okBim2PrjName));

	std::string okBim3PrjName = "okBIM3Test";
	EXPECT_EQ(REPOERR_OK,
		testUpload(mongoDbName, okBim3PrjName, "RepoModelImport/BrickWalls_bim3.bim"));
	EXPECT_TRUE(projectExists(mongoDbName, okBim3PrjName));

	std::string okBim4PrjName = "okBIM4Test";
	EXPECT_EQ(REPOERR_OK,
		testUpload(mongoDbName, okBim4PrjName, "RepoModelImport/wall_section_bim4.bim"));
	EXPECT_TRUE(projectExists(mongoDbName, okBim4PrjName));

	std::string corrTxtrBim3PrjName = "corruptedTextureBIM3Test";
	EXPECT_EQ(REPOERR_LOAD_SCENE_MISSING_TEXTURE,
		testUpload(mongoDbName, corrTxtrBim3PrjName, "RepoModelImport/BrickWalls_bim3_CorruptedTextureField.bim"));
	EXPECT_TRUE(projectExists(mongoDbName, corrTxtrBim3PrjName));

	std::string corrMatBim3PrjName = "corruptedMaterialBIM3Test";
	EXPECT_EQ(REPOERR_LOAD_SCENE_MISSING_NODES,
		testUpload(mongoDbName, corrMatBim3PrjName, "RepoModelImport/BrickWalls_bim3_MissingTexture.bim"));
	EXPECT_TRUE(projectExists(mongoDbName, corrMatBim3PrjName));
}

TEST(RepoClientTest, UploadTestIFC)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	//Upload IFCFile
	std::string ifcUpload = produceUploadArgs(db, "ifcTest", getDataPath(ifcModel));
	EXPECT_EQ((int)REPOERR_OK, runProcess(ifcUpload));
	EXPECT_TRUE(projectExists(db, "ifcTest"));

	std::string ifc4Upload = produceUploadArgs(db, "ifc4Test", getDataPath(ifc4Model));
	EXPECT_EQ((int)REPOERR_OK, runProcess(ifc4Upload));
	EXPECT_TRUE(projectExists(db, "ifc4Test"));
}

TEST(RepoClientTest, UploadTestDGN)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	//Upload DGN file
	std::string dgnUpload = produceUploadArgs(db, "dgnTest", getDataPath(dgnModel));
	EXPECT_EQ((int)REPOERR_OK, runProcess(dgnUpload));
	EXPECT_TRUE(projectExists(db, "dgnTest"));
}

TEST(RepoClientTest, UploadTestDWG)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	//Upload DWG file
	std::string dwgUpload = produceUploadArgs(db, "dwgTest", getDataPath(dwgModel));
	EXPECT_EQ((int)REPOERR_OK, runProcess(dwgUpload));
	EXPECT_TRUE(projectExists(db, "dwgTest"));
}

TEST(RepoClientTest, UploadTestDXF)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	//Upload XF file
	std::string dxfUpload = produceUploadArgs(db, "dxfTest", getDataPath(dxfModel));
	EXPECT_EQ((int)REPOERR_OK, runProcess(dxfUpload));
	EXPECT_TRUE(projectExists(db, "dxfTest"));
}

TEST(RepoClientTest, UploadTestRVT)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	//Upload RVT file
	std::string rvtUpload = produceUploadArgs(db, "rvtTest", getDataPath(rvtModel));
	EXPECT_EQ((int)REPOERR_LOAD_SCENE_MISSING_TEXTURE, runProcess(rvtUpload));
	EXPECT_TRUE(projectExists(db, "rvtTest"));

	//Upload RVT file with texture directory set
	std::string texturePath = "REPO_RVT_TEXTURES=" + getDataPath("textures");

	//Linux putenv takes in a char* instead of const char* - need a copy of the const char*
	char* texturePathEnv = new char[texturePath.size() + 1];
	strncpy(texturePathEnv, texturePath.c_str(), texturePath.size() + 1);

	putenv(texturePathEnv);
	std::string rvtUpload2 = produceUploadArgs(db, "rvtTest2", getDataPath(rvtModel));
	EXPECT_EQ((int)REPOERR_OK, runProcess(rvtUpload2));
	EXPECT_TRUE(projectExists(db, "rvtTest2"));

	//Upload RVT file with no valid 3D view
	std::string rvtUpload3 = produceUploadArgs(db, "rvtTest3", getDataPath(rvtNo3DViewModel));
	EXPECT_EQ((int)REPOERR_VALID_3D_VIEW_NOT_FOUND, runProcess(rvtUpload3));
	EXPECT_FALSE(projectExists(db, "rvtTest3"));
}

TEST(RepoClientTest, UploadTestRVT2021)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	//Upload RVT file
	std::string rvtUpload = produceUploadArgs(db, "rvtTest2021", getDataPath(rvtModel2021));
	EXPECT_EQ((int)REPOERR_OK, runProcess(rvtUpload));
	EXPECT_TRUE(projectExists(db, "rvtTest2021"));
}

TEST(RepoClientTest, UploadTestSPM)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	//commenting out 6.2 as we don't support this right now and it will crash.
	/*std::string spmUpload = produceUploadArgs(db, "synchroTest", getDataPath(synchroFile));
	EXPECT_EQ((int)REPOERR_OK, runProcess(spmUpload));
	EXPECT_TRUE(projectExists(db, "synchroTest"));*/

	//we don't support 6.1 - this will currently crash
	/*std::string spmUpload2 = produceUploadArgs(db, "synchroTest2", getDataPath(synchroOldVersion));
	EXPECT_EQ((int)REPOERR_UNSUPPORTED_VERSION, runProcess(spmUpload2));
	EXPECT_FALSE(projectExists(db, "synchroTest2"));*/

	//we also support 6.3
	std::string spmUpload3 = produceUploadArgs(db, "synchroTest3", getDataPath(synchroVersion6_3));
	EXPECT_EQ((int)REPOERR_OK, runProcess(spmUpload3));
	EXPECT_TRUE(projectExists(db, "synchroTest3"));

	std::string spmUpload4 = produceUploadArgs(db, "synchroTest4", getDataPath(synchroVersion6_4));
	EXPECT_EQ((int)REPOERR_OK, runProcess(spmUpload4));
	EXPECT_TRUE(projectExists(db, "synchroTest4"));
}

TEST(RepoClientTest, UploadTestRVTRegressionTests)
{
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	// Regression tests for fixed bugs
	std::string rvtUpload4 = produceUploadArgs(db, "rvtTest4", getDataPath(rvtRoofTest));
	EXPECT_EQ((int)REPOERR_LOAD_SCENE_MISSING_TEXTURE, runProcess(rvtUpload4));
	EXPECT_TRUE(projectExists(db, "rvtTest4"));

	std::string rvtUpload5 = produceUploadArgs(db, "rvtTest5", getDataPath(rvtMeta1));
	EXPECT_EQ((int)REPOERR_LOAD_SCENE_MISSING_TEXTURE, runProcess(rvtUpload5));
	EXPECT_TRUE(projectExists(db, "rvtTest5"));

	std::string rvtUpload6 = produceUploadArgs(db, "rvtTest6", getDataPath(rvtMeta2));
	EXPECT_EQ((int)REPOERR_OK, runProcess(rvtUpload6));
	EXPECT_TRUE(projectExists(db, "rvtTest6"));

	std::string rvtUpload7 = produceUploadArgs(db, "rvtTest7", getDataPath(rvtHouse));
	EXPECT_EQ((int)REPOERR_LOAD_SCENE_MISSING_TEXTURE, runProcess(rvtUpload7));
	EXPECT_TRUE(projectExists(db, "rvtTest7"));
}

TEST(RepoClientTest, UploadTestMissingFieldsInJSON)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));
	std::string db = "stUpload";

	//JSON AS argument
	//Empty JSON
	EXPECT_EQ((int)REPOERR_LOAD_SCENE_FAIL, runProcess(produceUploadFileArgs(getDataPath(emptyFile))));
	EXPECT_EQ((int)REPOERR_LOAD_SCENE_FAIL, runProcess(produceUploadFileArgs(getDataPath(importNoFile))));
	EXPECT_EQ((int)REPOERR_LOAD_SCENE_FAIL, runProcess(produceUploadFileArgs(getDataPath(emptyJSONFile))));
	EXPECT_EQ((int)REPOERR_MODEL_FILE_READ, runProcess(produceUploadFileArgs(getDataPath(importbadDir))));
	EXPECT_EQ((int)REPOERR_LOAD_SCENE_FAIL, runProcess(produceUploadFileArgs(getDataPath(importbadDir2))));
	EXPECT_EQ((int)REPOERR_LOAD_SCENE_FAIL, runProcess(produceUploadFileArgs(getDataPath(importNoDatabase))));
	EXPECT_EQ((int)REPOERR_LOAD_SCENE_FAIL, runProcess(produceUploadFileArgs(getDataPath(importNoDatabase2))));
	EXPECT_EQ((int)REPOERR_LOAD_SCENE_FAIL, runProcess(produceUploadFileArgs(getDataPath(importNoProject))));
	EXPECT_EQ((int)REPOERR_LOAD_SCENE_FAIL, runProcess(produceUploadFileArgs(importNoProject2)));
}

TEST(RepoClientTest, UploadTestOwner)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));

	EXPECT_EQ((int)REPOERR_OK, runProcess(produceUploadFileArgs(getDataPath(importNoOwner))));
	EXPECT_TRUE(projectExists("testDB", importNoOwnerPro));
	EXPECT_TRUE(projectSettingsCheck("testDB", importNoOwnerPro, REPO_GTEST_DBUSER, "thisTag", "MyUpload"));
	EXPECT_EQ((int)REPOERR_OK, runProcess(produceUploadFileArgs(getDataPath(importNoOwner2))));
	EXPECT_TRUE(projectExists("testDB", importNoOwnerPro2));
	EXPECT_TRUE(projectSettingsCheck("testDB", importNoOwnerPro2, REPO_GTEST_DBUSER, "thisTag", "MyUpload"));
}

TEST(RepoClientTest, CreateFedTest)
{
	//this ensures we can run processes
	ASSERT_TRUE(system(nullptr));

	//Test Bad FilePath
	std::string badFilePath = produceCreateFedArgs("nonExistentFile.json");
	EXPECT_EQ((int)REPOERR_FED_GEN_FAIL, runProcess(badFilePath));

	//Test Completely empty file
	std::string emptyFilePath = produceCreateFedArgs(getDataPath(emptyFile));
	EXPECT_EQ((int)REPOERR_FED_GEN_FAIL, runProcess(emptyFilePath));

	//Test json file with {}
	std::string empty2FilePath = produceCreateFedArgs(getDataPath(emptyJSONFile));
	EXPECT_EQ((int)REPOERR_FED_GEN_FAIL, runProcess(empty2FilePath));

	//Test json file with no sub projects
	std::string noSPFilePath = produceCreateFedArgs(getDataPath(noSubProjectJSONFile));
	EXPECT_EQ((int)REPOERR_FED_GEN_FAIL, runProcess(noSPFilePath));
	EXPECT_FALSE(projectExists(genFedDB, genFedNoSubProName));

	//Test json file with empty string as database name
	std::string noDBFilePath = produceCreateFedArgs(getDataPath(noDbNameJSONFile));
	EXPECT_EQ((int)REPOERR_FED_GEN_FAIL, runProcess(noDBFilePath));

	//Test json file with empty string as project name
	std::string noProFilePath = produceCreateFedArgs(getDataPath(noProNameJSONFile));
	EXPECT_EQ((int)REPOERR_FED_GEN_FAIL, runProcess(noProFilePath));

	//Test badly formatted JSON file
	std::string invalidJSONFilePath = produceCreateFedArgs(getDataPath(invalidJSONFile));
	EXPECT_EQ((int)REPOERR_FED_GEN_FAIL, runProcess(invalidJSONFilePath));

	//Test success
	std::string goodFilePath = produceCreateFedArgs(getDataPath(validGenFedJSONFile));
	EXPECT_EQ((int)REPOERR_OK, runProcess(goodFilePath));
	EXPECT_TRUE(projectExists(genFedDB, genFedSuccessName));
}

TEST(RepoClientTest, GetFileTest)
{
	EXPECT_EQ((int)REPOERR_GET_FILE_FAILED, runProcess(produceGetFileArgs(".", "nonExistent1", "nonExistent2")));
	EXPECT_EQ((int)REPOERR_GET_FILE_FAILED, runProcess(produceGetFileArgs(".", REPO_GTEST_DBNAME1, "nonExistent2")));

	EXPECT_EQ((int)REPOERR_OK, runProcess(produceGetFileArgs(".", "sampleDataRW", "cube")));
	EXPECT_TRUE(fileExists(getFileFileName));
	EXPECT_TRUE(filesCompare(getFileFileName, getDataPath("cube.obj")));
}

TEST(RepoClientTest, GenStashTest)
{
	repo::RepoController *controller = new repo::RepoController();
	std::string errMsg;
	repo::RepoController::RepoToken *token = initController(controller);
	repo::lib::RepoUUID stashRoot;
	if (token)
	{
		auto scene = controller->fetchScene(token, "sampleDataRW", "cube");
		if (scene)
		{
			stashRoot = scene->getRoot(repo::core::model::RepoScene::GraphType::OPTIMIZED)->getUniqueID();
			delete scene;
		}
	}

	EXPECT_EQ((int)REPOERR_OK, runProcess(produceGenStashArgs("sampleDataRW", "cube", "src")));
	EXPECT_EQ((int)REPOERR_OK, runProcess(produceGenStashArgs("sampleDataRW", "cube", "tree")));
	EXPECT_EQ((int)REPOERR_OK, runProcess(produceGenStashArgs("sampleDataRW", "cube", "gltf")));
	EXPECT_EQ((int)REPOERR_OK, runProcess(produceGenStashArgs("sampleDataRW", "cube", "repo")));

	if (token)
	{
		auto scene = controller->fetchScene(token, "sampleDataRW", "cube");
		if (scene)
		{
			EXPECT_NE(scene->getRoot(repo::core::model::RepoScene::GraphType::OPTIMIZED)->getUniqueID(), stashRoot);

			delete scene;
		}
	}

	controller->disconnectFromDatabase(token);
	EXPECT_EQ((int)REPOERR_STASH_GEN_FAIL, runProcess(produceGenStashArgs("blash", "blah", "tree")));
	EXPECT_EQ((int)REPOERR_STASH_GEN_FAIL, runProcess(produceGenStashArgs("blash", "blah", "src")));
	EXPECT_EQ((int)REPOERR_STASH_GEN_FAIL, runProcess(produceGenStashArgs("blash", "blah", "gltf")));

	delete controller;
}
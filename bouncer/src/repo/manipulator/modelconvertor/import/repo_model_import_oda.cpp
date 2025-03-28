#include "repo_model_import_oda.h"
#include "../../../core/model/bson/repo_bson_factory.h"
#include "../../../error_codes.h"

#ifdef ODA_SUPPORT
#include <OdaCommon.h>
#include <Gs/GsBaseInclude.h>
#include "odaHelper/helper_functions.h"
#endif

using namespace repo::manipulator::modelconvertor;

const std::string OdaModelImport::supportedExtensions = ".dgn.rvt.rfa.dwg.dxf";

OdaModelImport::~OdaModelImport()
{
}

static repo_material_t createDefaultMaterial() {
	repo_material_t matStruct;
	matStruct.diffuse = { 1, 1, 1 };
	matStruct.opacity = 1;
}

bool OdaModelImport::isSupportedExts(const std::string &testExt)
{
	std::string lowerExt(testExt);
	std::transform(lowerExt.begin(), lowerExt.end(), lowerExt.begin(), ::tolower);

	return supportedExtensions.find(lowerExt) != std::string::npos;
}

repo::core::model::RepoScene* OdaModelImport::generateRepoScene(uint8_t &errMsg)
{
	repo::core::model::RepoScene *scene = nullptr;
#ifdef ODA_SUPPORT
	repoInfo << "Constructing Repo Scene...";
	const repo::core::model::RepoNodeSet dummy;
	auto rootNode = geoCollector.createRootNode();
	auto meshSet = geoCollector.getMeshNodes(rootNode);
	if (meshSet.size()) {
		repoInfo << "Get material nodes... ";

		repo::core::model::RepoNodeSet materialSet, textureSet;

		geoCollector.getMaterialAndTextureNodes(materialSet, textureSet);
		auto transSet = geoCollector.getTransformationNodes();
		auto metaSet = geoCollector.getMetaNodes();
		repoInfo << "Nodes count - Trans: " << transSet.size() << " meshes: " << meshSet.size() << " materials: " << materialSet.size() << " metadata: " << metaSet.size();
		scene = new repo::core::model::RepoScene({ filePath }, dummy, meshSet, materialSet, metaSet, textureSet, transSet);
		if (geoCollector.hasMissingTextures())
			scene->setMissingTexture();
		scene->setWorldOffset(geoCollector.getModelOffset());
		repoInfo << "Repo Scene constructed.";
	}
	else {
		repoError << "No meshes generated";
		scene = new repo::core::model::RepoScene({ filePath }, dummy, dummy, dummy, dummy, dummy, dummy);
	}

#else
	errMsg = REPOERR_ODA_UNAVAILABLE;
#endif
	return scene;
}
bool OdaModelImport::importModel(std::string filePath, uint8_t &err)
{
#ifdef ODA_SUPPORT
	this->filePath = filePath;
	repoInfo << " ==== Importing with Teigha Library [" << filePath << "] ====";
	odaProcessor = odaHelper::FileProcessor::getFileProcessor(filePath, &geoCollector);
	shouldReduce = odaProcessor->shouldApplyReduction;
	bool success = false;
	err = REPOERR_OK;
	try {
		err = odaProcessor->readFile();
		if (err == REPOERR_OK) {
			err = geoCollector.getErrorCode(); // the outermost error codes should take precedence as they could cause inner errors
		}
		success = odaProcessor != nullptr && err == REPOERR_OK;
	}
	catch (std::exception& ex) {
		success = false;
		err = REPOERR_LOAD_SCENE_FAIL;
	}

	return success;
#else
	//ODA support not compiled in.
	err = REPOERR_ODA_UNAVAILABLE;
	return false;
#endif
}
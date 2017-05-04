
#include "stdafx.h"
#include "resourcemanager.h"
#include "../importer/modelimporter.h"
#include "../base/material.h"
#include "../model_collada/model_collada.h"
#include "task_resource.h"


using namespace graphic;


cResourceManager::cResourceManager() :
	m_mediaDirectory("../media/")
	, m_loadId(0)
{
}

cResourceManager::~cResourceManager()
{
	Clear();
}


// load model file
sRawMeshGroup* cResourceManager::LoadRawMesh( const string &fileName )
{
	RETV(fileName.empty(), NULL);

	if (sRawMeshGroup *data = FindModel(fileName))
		return data;

	sRawMeshGroup *meshes = new sRawMeshGroup;
	meshes->name = fileName;

	if (!importer::ReadRawMeshFile(fileName, *meshes))
	{
		string newPath;
		if (common::FindFile(fileName, m_mediaDirectory, newPath))
		{
			if (!importer::ReadRawMeshFile(newPath, *meshes))
			{
				goto error;
			}
		}
		else
		{
			goto error;
		}
	}

	InsertRawMesh(meshes);
	return meshes;

error:
	delete meshes;
	dbg::ErrLog("Error!! LoadRawMesh() [%s] \n", fileName.c_str());
	return NULL;
}


// �ܺο��� �ε��� �޽��� �����Ѵ�.
bool cResourceManager::InsertRawMesh(sRawMeshGroup *meshes)
{
	RETV(!meshes, false);

	if (sRawMeshGroup *data = FindModel(meshes->name))
		return false;

	// �޽� �̸� ���� fileName::meshName
	for (u_int i = 0; i < meshes->meshes.size(); ++i)// auto &mesh : meshes->meshes)
	{
		sRawMesh &mesh = meshes->meshes[i];

		mesh.name = meshes->name + "::" + mesh.name;
		if (mesh.mtrlId >= 0)
		{ // ���͸��� ����.
			mesh.mtrl = meshes->mtrls[mesh.mtrlId];
		}
	}

	m_meshes[meshes->name] = meshes;
	return true;
}


// load collada model file
sRawMeshGroup2* cResourceManager::LoadRawMesh2(const string &fileName)
{
	RETV(fileName.empty(), NULL);

	if (sRawMeshGroup2 *data = FindModel2(fileName))
		return data;

	cColladaLoader loader;
	if (!loader.Create(fileName))
		return NULL;

	if (loader.m_rawMeshes)
		m_meshes2[fileName] = loader.m_rawMeshes;
	if (loader.m_rawAnies)
	{
		loader.m_rawMeshes->animationName = loader.m_rawAnies->name;
		m_anies[loader.m_rawAnies->name] = loader.m_rawAnies;
	}

	return loader.m_rawMeshes;
}


cXFileMesh* cResourceManager::LoadXFile(cRenderer &renderer, const string &fileName)
{
	if (cXFileMesh *p = FindXFile(fileName))
		return p;

	cXFileMesh *mesh = new cXFileMesh;
	if (!mesh->Create(renderer, fileName, false, true))
	{
		delete mesh;
		return NULL;
	}

	m_xfiles[fileName] = mesh;
	return mesh;
}


std::pair<bool, cXFileMesh*> cResourceManager::LoadXFileParallel(cRenderer &renderer, const string &fileName)
{
	if (cXFileMesh *p = FindXFile(fileName))
		return{ true, p };

	m_loadThread.AddTask(new cTaskXFileLoader(++m_loadId, &renderer, fileName));
	if (!m_loadThread.IsRun())
		m_loadThread.Start();

	AutoCSLock cs(m_cs);
	m_xfiles[fileName] = NULL;
	return{ true, NULL };
}


void cResourceManager::InsertXFileModel(const string &fileName, cXFileMesh *p)
{
	AutoCSLock cs(m_cs);
	m_xfiles[fileName] = p;
}


cColladaModel* cResourceManager::LoadColladaModel( cRenderer &renderer
	, const string &fileName
)
{
	if (cColladaModel *p = FindColladaModel(fileName))
		return p;

	AutoCSLock cs(m_cs);
	cColladaModel *model = new cColladaModel;
	if (!model->Create(renderer, fileName))
	{
		delete model;
		return NULL;
	}

	m_colladaModels[fileName] = model;
	return model;
}


// Load Parallel Collada file
std::pair<bool, cColladaModel*> cResourceManager::LoadColladaModelParallel(cRenderer &renderer, const string &fileName)
{
	if (cColladaModel *p = FindColladaModel(fileName))
		return{ true, p };

	m_loadThread.AddTask( new cTaskColladaLoader(++m_loadId, &renderer, fileName) );
	if (!m_loadThread.IsRun())
		m_loadThread.Start();

	AutoCSLock cs(m_cs);
	m_colladaModels[fileName] = NULL;
	return{ true, NULL };
}


// Set Model Pointer If Finish Parallel Load 
void cResourceManager::InsertColladaModel(const string &fileName, cColladaModel *p)
{
	AutoCSLock cs(m_cs);
	m_colladaModels[fileName] = p;
}


cShadowVolume* cResourceManager::LoadShadow(cRenderer &renderer, const string &fileName)
{
	if (cShadowVolume *p = FindShadow(fileName))
		return p;

	cShadowVolume *shadow = NULL;
	cColladaModel *collada = FindColladaModel(fileName);
	cXFileMesh *xfile = FindXFile(fileName);
	if (!collada && !xfile)
		return NULL;
	if (collada && xfile)
	{
		assert(0);
		return NULL;
	}

	if (collada)
	{
		shadow = new cShadowVolume();
		if (!collada->m_meshes.empty())
		{
			cMesh2 *mesh = collada->m_meshes[0];
			if (!shadow->Create(renderer, mesh->m_buffers->m_vtxBuff, mesh->m_buffers->m_idxBuff))
			{
				SAFE_DELETE(shadow);
			}
		}
	}
	else if (xfile)
	{
		shadow = new cShadowVolume();
		if (!shadow->Create(renderer, xfile->m_mesh, false))
		{
			SAFE_DELETE(shadow);
		}
	}

	AutoCSLock cs(m_csShadow);
	m_shadows[fileName] = shadow;
	return shadow;
}


std::pair<bool, cShadowVolume*> cResourceManager::LoadShadowParallel(cRenderer &renderer
	, const string &fileName
)
{
	if (cShadowVolume *p = FindShadow(fileName))
		return{ true, p };

	cColladaModel *collada = FindColladaModel(fileName);
	cXFileMesh *xfile = FindXFile(fileName);
	if (!collada && !xfile)
		return{ false, NULL };
	if (collada && xfile)
	{
		assert(0);
		return{ false, NULL };
	}

	{
		AutoCSLock cs(m_csShadow);
		m_shadows[fileName] = NULL;
	}

	m_loadThread.AddTask(new cTaskShadowLoader(++m_loadId, &renderer, fileName, collada, xfile));
	if (!m_loadThread.IsRun())
		m_loadThread.Start();

	return{ true, NULL };
}


void cResourceManager::InsertShadow(const string &fileName, cShadowVolume *p)
{
	AutoCSLock cs(m_csShadow);
	m_shadows[fileName] = p;
}


// �ִϸ��̼� ���� �ε�.
sRawAniGroup* cResourceManager::LoadAnimation( const string &fileName )
{
	RETV(fileName.empty(), NULL);

	if (sRawAniGroup *data = FindAnimation(fileName))
		return data;

	sRawAniGroup *anies = new sRawAniGroup;
	anies->name = fileName;

	if (!importer::ReadRawAnimationFile(fileName, *anies))
	{
		string newPath;
		if (common::FindFile(fileName, m_mediaDirectory, newPath))
		{
			if (!importer::ReadRawAnimationFile(newPath, *anies))
			{
				goto error;
			}
		}
		else
		{
			goto error;
		}
	}

	LoadAnimation(anies);
	return anies;

error:
	delete anies;
	return NULL;
}


// Register Animation Information
bool cResourceManager::LoadAnimation(sRawAniGroup *anies)
{
	RETV(!anies, false);

	m_anies[anies->name] = anies;
	return true;
}


// meshName�� �ش��ϴ� �޽����۸� �����Ѵ�.
cMeshBuffer* cResourceManager::LoadMeshBuffer(cRenderer &renderer, const string &meshName)
{
	if (cMeshBuffer *data = FindMeshBuffer(meshName))
		return data;

	string fileName = meshName;
	fileName.erase(meshName.find("::"));

	if (sRawMeshGroup *meshes = LoadRawMesh(fileName))
	{
		for each (auto &rawMesh in meshes->meshes)
		{
			if (meshName == rawMesh.name)
			{
				cMeshBuffer *buffer = new cMeshBuffer(renderer, rawMesh);
				m_mesheBuffers[ meshName] = buffer;
				return buffer;
			}
		}
	}
	
	return NULL;
}


// rawMesh ������ MeshBuffer�� �����Ѵ�.
cMeshBuffer* cResourceManager::LoadMeshBuffer(cRenderer &renderer, const sRawMesh &rawMesh)
{
	if (cMeshBuffer *data = FindMeshBuffer(rawMesh.name))
		return data;

	cMeshBuffer *buffer = new cMeshBuffer(renderer, rawMesh);
	m_mesheBuffers[rawMesh.name] = buffer;
	return buffer;
}


// meshName���� �޽����۸� ã�� �����Ѵ�.
cMeshBuffer* cResourceManager::FindMeshBuffer( const string &meshName )
{
	auto it = m_mesheBuffers.find(meshName);
	if (m_mesheBuffers.end() == it)
		return NULL; // not exist

	return it->second;
}


// find model data
sRawMeshGroup* cResourceManager::FindModel( const string &fileName )
{
	auto it = m_meshes.find(fileName);
	if (m_meshes.end() == it)
		return NULL; // not exist

	//if (m_reLoadFile.end() != m_reLoadFile.find(fileName))
	//{ // ���ε��� �����̶�� �����ϰ� ���� �� ó�� ó���Ѵ�.
	//	delete it->second;
	//	m_meshes.erase(fileName);
	//	m_reLoadFile.erase(fileName);
	//	return NULL;
	//}

	return it->second;
}


// find model data
sRawMeshGroup2* cResourceManager::FindModel2(const string &fileName)
{
	auto it = m_meshes2.find(fileName);
	if (m_meshes2.end() == it)
		return NULL; // not exist
	return it->second;
}


cXFileMesh* cResourceManager::FindXFile(const string &fileName)
{
	AutoCSLock cs(m_cs);

	auto it = m_xfiles.find(fileName);
	if (m_xfiles.end() != it)
		return it->second;
	return NULL;
}


cColladaModel * cResourceManager::FindColladaModel(const string &fileName)
{
	AutoCSLock cs(m_cs);
	auto it = m_colladaModels.find(fileName);
	if (m_colladaModels.end() != it)
		return it->second;
	return NULL;
}


cShadowVolume* cResourceManager::FindShadow(const string &fileName)
{
	AutoCSLock cs(m_csShadow);
	auto it = m_shadows.find(fileName);
	if (m_shadows.end() != it)
		return it->second;
	return NULL;
}


// find animation data
sRawAniGroup* cResourceManager::FindAnimation( const string &fileName )
{
	auto it = m_anies.find(fileName);
	if (m_anies.end() == it)
		return NULL; // not exist

	//if (m_reLoadFile.end() != m_reLoadFile.find(fileName))
	//{ // ���ε��� �����̶�� �����ϰ� ���� �� ó�� ó���Ѵ�.
	//	delete it->second;
	//	m_meshes.erase(fileName);
	//	m_reLoadFile.erase(fileName);
	//	return NULL;
	//}

	return it->second;
}


// �ؽ��� �ε�.
cTexture* cResourceManager::LoadTexture(cRenderer &renderer, const string &fileName
	,  const bool isSizePow2 //=true
	, const bool isRecursive //= true
)
{
	const string whiteTexture = "model/white.dds";

	if (cTexture *p = FindTexture(fileName))
		return p;

	string key = fileName;
	cTexture *texture = NULL;// new cTexture();

	//if (!texture->Create(renderer, fileName, isSizePow2))
	if (common::IsFileExist(fileName))
	{
		texture = new cTexture();
		if (!texture->Create(renderer, fileName, isSizePow2))
		{
			if (fileName == whiteTexture) // this file must loaded
				assert(0);
		}
	}
	else
	{
		string newPath;
		if (common::FindFile(fileName, m_mediaDirectory, newPath))
		{
			if (isRecursive)
			{
				if (texture = cResourceManager::LoadTexture(renderer, newPath, isSizePow2, false))
					return texture;
			}
			else
			{
				//cTexture *texture = NULL;// new cTexture();
				if (common::IsFileExist(newPath))
				{
					key = newPath;
					texture = new cTexture();
					if (!texture->Create(renderer, newPath, isSizePow2))
					{
						if (newPath == whiteTexture) // this file must loaded
							assert(0);
					}


					//{
						// last load white.dds texture
						//if (!texture->Create(renderer, "model/white.dds", isSizePow2))
						//{
						//delete texture;
						//return false;
						//}
						//texture = cResourceManager::LoadTexture(renderer, "model/white.dds", false);
					//}
				}

				//if (!texture->Create(renderer, newPath, isSizePow2))
				//{
				//	// last load white.dds texture
				//	//if (!texture->Create(renderer, "model/white.dds", isSizePow2))
				//	//{
				//		delete texture;
				//		//return false;
				//	//}
				//	texture = cResourceManager::LoadTexture(renderer, "model/white.dds", false);
				//}
			}
		}
	}

	if (texture && texture->IsLoaded())
	{
		m_textures[key] = texture;
		return texture;
	}
	else
	{
		// load error
		if (!texture)
		{  // Not Exist File
			return cResourceManager::LoadTexture(renderer, whiteTexture, isSizePow2, false);
		}

		// last load white.dds texture
		if (!texture->IsLoaded())
		{ // File Exist, but Load Error
			delete texture;
			return cResourceManager::LoadTexture(renderer, whiteTexture, isSizePow2, false);
		}
	}

	return NULL;
}


// �ؽ��� �ε�.
// fileName �� �ش��ϴ� ������ ���ٸ�, "../media/" + dirPath  ��ο��� ������ ã�´�.
cTexture* cResourceManager::LoadTexture(cRenderer &renderer, const string &dirPath, const string &fileName
	, const bool isSizePow2 //= true
	, const bool isRecursive //= true
)	
{
	const string whiteTexture = "model/white.dds";

	if (cTexture *p = FindTexture(fileName))
		return p;

	string key = fileName;
	cTexture *texture = NULL;// new cTexture();
	if (common::IsFileExist(fileName))
	{
		texture = new cTexture();
		texture->Create(renderer, fileName, isSizePow2);
	}
	else
	{
		string newPath;
		string searchPath = m_mediaDirectory + dirPath;
		if (searchPath.empty())
			searchPath = ".";

		key = newPath;
		if (common::FindFile(GetFileName(fileName), searchPath + "/", newPath))
		{
			if (isRecursive)
			{
				if (texture = cResourceManager::LoadTexture(renderer, newPath, isSizePow2, false))
					return texture;
			}
			else
			{
				if (common::IsFileExist(newPath))
				{
					texture = new cTexture();
					texture->Create(renderer, newPath, isSizePow2);
				}
			}
		}
	}

	if (texture && texture->IsLoaded())
	{
		m_textures[key] = texture;
		return texture;
	}
	else
	{
		// load error
		if (!texture)
		{  // Not Exist File
			return cResourceManager::LoadTexture(renderer, whiteTexture, isSizePow2, false);
		}

		// last load white.dds texture
		if (!texture->IsLoaded())
		{ // File Exist, but Load Error
			delete texture;
			return cResourceManager::LoadTexture(renderer, whiteTexture, isSizePow2, false);
		}
	}

	return NULL;


	//if (!texture->Create(renderer, fileName, isSizePow2))
	//{
	//	string newPath;
	//	string searchPath = m_mediaDirectory + dirPath;
	//	if (searchPath.empty())
	//		searchPath = ".";

	//	if (common::FindFile(GetFileName(fileName), searchPath + "/", newPath))
	//	{
	//		if (!texture->Create(renderer, newPath, isSizePow2))
	//		{
	//			delete texture;
	//			return false;
	//		}
	//	}
	//}

	//// last load white.dds texture
	//if (!texture->m_texture)
	//{
	//	delete texture;
	//	texture = cResourceManager::LoadTexture(renderer, "model/white.dds");
	//	//texture->Create(renderer, m_mediaDirectory + "/texture/white.dds", isSizePow2);
	//}

	//m_textures[ fileName] = texture;
	//return texture;
}


// ���̴� �ε�.
cShader* cResourceManager::LoadShader(cRenderer &renderer, const string &fileName, const bool isReload)
// isReload=false
{
	if (cShader *p = FindShader(fileName))
	{
		if (isReload)
		{
			delete p;
		}
		else
		{
			return p;
		}
	}

	cShader *shader = new cShader();
	if (!shader->Create(renderer, fileName, "TShader", false))
	{
		string newPath;
		string searchPath = m_mediaDirectory;
		if (searchPath.empty())
			searchPath = "./";

		if (common::FindFile(fileName, searchPath, newPath))
		{
			if (!shader->Create(renderer, newPath, "TShader"))
			{
				delete shader;
				return NULL; // ���� ����.
			}
		}
		else
		{
			string msg = fileName + " ������ �������� �ʽ��ϴ�.";
			MessageBoxA( NULL, msg.c_str(), "ERROR", MB_OK);
		}
	}

	m_shaders[ GetFileName(fileName)] = shader;
	return shader;
}


// �ؽ��� ã��.
cTexture* cResourceManager::FindTexture( const string &fileName )
{
	auto it = m_textures.find(fileName);
	if (m_textures.end() == it)
		return NULL; // not exist
	return it->second;
}


// ���̴� ã��.
cShader* cResourceManager::FindShader( const string &fileName )
{
	auto it = m_shaders.find(GetFileName(fileName));
	if (m_shaders.end() == it)
		return NULL; // not exist
	return it->second;
}


// media �������� fileName �� �ش��ϴ� ������ �����Ѵٸ�,
// ��ü ��θ� �����Ѵ�. ������ ã�� ���� �����̸��� ���Ѵ�.
string cResourceManager::FindFile( const string &fileName )
{
	string newPath;
	if (common::FindFile(fileName, m_mediaDirectory, newPath))
	{
		return newPath;
	}

	return ""; //empty string
}


// remove all data
void cResourceManager::Clear()
{
	// remove raw mesh
	for each (auto kv in m_meshes)
	{
		delete kv.second;
	}
	m_meshes.clear();

	// remove raw mesh2
	for each (auto kv in m_meshes2)
	{
		delete kv.second;
	}
	m_meshes2.clear();

	// remove raw m_xfiles
	{
		AutoCSLock cs(m_cs);
		for each (auto kv in m_xfiles)
		{
			delete kv.second;
		}
		m_xfiles.clear();
	}

	// remove raw collada models
	{
		AutoCSLock cs(m_cs);
		for each (auto kv in m_colladaModels)
		{
			delete kv.second;
		}
		m_colladaModels.clear();
	}

	// remove shadow
	{
		AutoCSLock cs(m_csShadow);
		for each (auto kv in m_shadows)
		{
			delete kv.second;
		}
		m_shadows.clear();
	}

	// remove texture
	for each (auto kv in m_textures)
	{
		delete kv.second;
	}
	m_textures.clear();

	// remove raw ani
	for each (auto kv in m_anies)
	{
		delete kv.second;
	}
	m_anies.clear();

	// remove shader
	for each (auto kv in m_shaders)
	{
		delete kv.second;
	}
	m_shaders.clear();

	// remove mesh buffer
	for each (auto kv in m_mesheBuffers)
	{
		delete kv.second;
	}
	m_mesheBuffers.clear();
}


// ���� ������ �����Ѵ�.
RESOURCE_TYPE::TYPE cResourceManager::GetFileKind( const string &fileName )
{
	return importer::GetFileKind(fileName);
}


// ���ϰ�� fileName�� media ������ ��� �ּҷ� �ٲ㼭 �����Ѵ�.
// ex)
// media : c:/project/media,  
// fileName : c:/project/media/terrain/file.txt
// result = ./terrain/file.txt
string cResourceManager::GetRelativePathToMedia( const string &fileName )
{
	const string mediaFullPath = common::GetFullFileName(m_mediaDirectory);
	const string fullFileName = common::GetFullFileName(fileName);
	const string relatePath = common::RelativePathTo( mediaFullPath, fullFileName);
	return relatePath;
}


// �̹� �ε��� ������ �������� �ʰ�, �ٽ� �ε��Ѵ�.
// �޽�, �ִϸ��̼Ǹ� �ش�ȴ�.
void cResourceManager::ReloadFile()
{
	// �ϴ� ��� ��.
	// �� ����� ����� �����Ϸ���, ��ü ������ �ʿ��ϴ�.
	// ������ �޸𸮸� �����ϴ� ��찡 �߻��Ѵ�.

	//for each (auto kv, m_meshes)
	//	m_reLoadFile.insert(kv.first);
	//for each (auto kv, m_anies)
	//	m_reLoadFile.insert(kv.first);
}


void cResourceManager::ReloadShader(cRenderer &renderer)
{
	for (auto p : m_shaders)
		p.second->Reload(renderer);
}


void cResourceManager::LostDevice()
{
	for (auto &p : m_textures)
		p.second->LostDevice();
	for (auto &p : m_shaders)
		p.second->LostDevice();
}


void cResourceManager::ResetDevice(cRenderer &renderer)
{
	for (auto &p : m_textures)
		p.second->ResetDevice(renderer);
	for (auto &p : m_shaders)
		p.second->ResetDevice(renderer);
}


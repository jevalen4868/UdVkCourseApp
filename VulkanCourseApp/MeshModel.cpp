
#include "MeshModel.h"

MeshModel::MeshModel() {
}

MeshModel::MeshModel(vector<Mesh> meshes) {
	_meshes = meshes;
	_model = glm::mat4(1.0f);
}

MeshModel::~MeshModel() {
}

size_t MeshModel::getMeshCount() {
	return _meshes.size();
}

Mesh *MeshModel::getMesh(size_t index) {
	if (index >= _meshes.size()) {
		throw std::runtime_error("Attempted to access invalid mesh index=" + index);
	}
	return &_meshes[index];
}

glm::mat4 MeshModel::getModel() {
	return _model;
}

void MeshModel::setModel(glm::mat4 model) {
	_model = model;
}

void MeshModel::destroyMeshModel() {
	for (auto &mesh : _meshes) {
		mesh.destroyBuffers();
	}
}

vector<string> MeshModel::LoadMaterials(const aiScene *scene) {
	// Create 1:1 sized list of textures.
	vector<string> textures(scene->mNumMaterials);
	for (size_t i{ 0 }; i < scene->mNumMaterials; i++) {
		// Get the mat.
		aiMaterial *mat{ scene->mMaterials[i] };

		// Init the text to empty string (will be replaced if tex exists.
		textures[i] = "";

		// Check for a diffuse texture (standard detail texture)
		if (mat->GetTextureCount(aiTextureType_DIFFUSE)) {
			// Get the path of the texture file.
			aiString path;
			if (mat->GetTexture(aiTextureType_DIFFUSE,
				0,
				&path) == AI_SUCCESS) {
				
				string pathStr{ path.data };
				// Cut off any dir info present.
				int idx = pathStr.rfind("\\");
				string fileName = pathStr.substr(idx + 1);

				idx = pathStr.rfind("/");
				fileName = pathStr.substr(idx + 1);

				textures[i] = fileName;
			}
		}
	}

	return textures;
}

std::vector<Mesh> MeshModel::LoadNode(VkPhysicalDevice physDev, VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, aiNode *node, const aiScene *scene, vector<int> matToTex) {
	vector<Mesh> meshes;
	// Go through each mesh at this node and create it, then add it to our meshes.
	for (size_t i{ 0 }; i < node->mNumMeshes; i++) {
		// Load mesh here.
		uint32_t meshId{ node->mMeshes[i] };
		meshes.push_back(
			LoadMesh(physDev, device, transferQueue, transferCommandPool, scene->mMeshes[meshId], scene, matToTex)
		);
	}

	// Go through each node attached to this node and append their meshes to this node's mesh list.
	for (size_t i{ 0 }; i < node->mNumChildren; i++) {
		vector<Mesh> newMeshes{
			LoadNode(physDev, device, transferQueue, transferCommandPool, node->mChildren[i], scene, matToTex)
		};

		meshes.insert(meshes.end(), newMeshes.begin(), newMeshes.end());
	}

	return meshes;
}

Mesh MeshModel::LoadMesh(VkPhysicalDevice physDev, VkDevice device, VkQueue transferQueue, VkCommandPool transferCommandPool, aiMesh *mesh, const aiScene *scene, vector<int> matToTex) {
	vector<Vertex> vertices(mesh->mNumVertices);
	vector<uint32_t> indices;

	// Go through each vertex and copy it across to our vertices.
	for (size_t i{ 0 }; i < mesh->mNumVertices; i++) {
		// Set position
		aiVector3t<ai_real> *vertex{ mesh->mVertices };
		vertices[i].pos = { vertex->x, vertex->y, vertex->z };

		// Set tex coords (if they exist).
		if (mesh->mTextureCoords[0]) {
			aiVector3t<ai_real> textureCoords{ mesh->mTextureCoords[0][i] };
			vertices[i].tex = {
				textureCoords.x, textureCoords.y
			};
		}
		else {
			vertices[i].tex = {
				0.0f, 0.0f
			};
		}

		// Set color (just use white for now)
		vertices[i].col = { 1.0f, 1.0f, 1.0f };
	}

	// Iterate over indices through faces and copy across.
	for (size_t i{ 0 }; i < mesh->mNumFaces; i++) {
		// Get a face.
		aiFace face{ mesh->mFaces[i] };

		// Go through faces indices and add to list.
		for (size_t j{ 0 }; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	// Create new mesh with details and return it.
	Mesh newMesh{
		physDev, device, transferQueue, transferCommandPool, &vertices, &indices, matToTex[mesh->mMaterialIndex]
	};

	// Return new mesh.
	return newMesh;
}


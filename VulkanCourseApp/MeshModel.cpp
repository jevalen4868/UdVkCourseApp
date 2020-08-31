
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

#ifdef NDEBUG
		printf("Texture Count aiTextureType_DIFFUSE\t=%i\n", mat->GetTextureCount(aiTextureType_DIFFUSE));
		printf("Texture Count aiTextureType_AMBIENT\t=%i\n", mat->GetTextureCount(aiTextureType_AMBIENT));
		printf("Texture Count aiTextureType_DISPLACEMENT=%i\n", mat->GetTextureCount(aiTextureType_DISPLACEMENT));
		printf("Texture Count aiTextureType_EMISSIVE\t=%i\n", mat->GetTextureCount(aiTextureType_EMISSIVE));
		printf("Texture Count aiTextureType_HEIGHT\t=%i\n", mat->GetTextureCount(aiTextureType_HEIGHT));
		printf("Texture Count aiTextureType_LIGHTMAP\t=%i\n", mat->GetTextureCount(aiTextureType_LIGHTMAP));
		printf("Texture Count aiTextureType_NONE\t=%i\n", mat->GetTextureCount(aiTextureType_NONE));
		printf("Texture Count aiTextureType_NORMALS\t=%i\n", mat->GetTextureCount(aiTextureType_NORMALS));
		printf("Texture Count aiTextureType_OPACITY\t=%i\n", mat->GetTextureCount(aiTextureType_OPACITY));
		printf("Texture Count aiTextureType_REFLECTION\t=%i\n", mat->GetTextureCount(aiTextureType_REFLECTION));
		printf("Texture Count aiTextureType_SPECULAR\t=%i\n", mat->GetTextureCount(aiTextureType_SPECULAR));
		printf("Texture Count aiTextureType_SHININESS\t=%i\n", mat->GetTextureCount(aiTextureType_SHININESS));
		printf("Texture Count aiTextureType_UNKNOWN\t=%i\n\n", mat->GetTextureCount(aiTextureType_UNKNOWN));
#endif
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
		vertices[i].pos = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

		// Set tex coords (if they exist).
		if (mesh->mTextureCoords[0]) {			
			vertices[i].tex = {
				mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y
			};
		}
		else {
			vertices[i].tex = {
				0.0f, 0.0f
			};
		}

		if (mesh->mColors[0]) {
			vertices[i].col = { mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b };
		}
		else {
			// Set color (just use white for now)
			vertices[i].col = { 1.0f, 1.0f, 1.0f };
		}

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


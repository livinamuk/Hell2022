#include "Model.h"
#include "Helpers/Util.h"
#include "Header.h"
#include <assert.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobj/tiny_obj_loader.h"


Model::Model()
{
}

Model::Model(const char* filepath, bool createCollisionMesh)
{
	this->m_filePath = std::string(filepath);
	this->name = Util::FileNameFromPath(filepath);
	this->m_fileType = Util::FileTypeFromPath(filepath);
	ReadFromDisk(createCollisionMesh);
	LoadMeshDataToGL();
}

Model::~Model()
{
}

void Model::ReadFromDisk(bool createCollisionMesh)
{
	std::cout << "Loading obj model: " << this->name << "\n";

	// Check file exists
	if (!FileExists(this->m_filePath.c_str()))
		std::cout << this->m_filePath.c_str() << " does not exist!\n";

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;
			
	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, this->m_filePath.c_str())) {
		std::cout << "Crashed loading " << this->m_filePath.c_str() << "\n";
		//return;
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

	for (const auto& shape : shapes)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;

		for (int i=0; i < shape.mesh.indices.size(); i++) {
//		for (const auto& index : shape.mesh.indices) {
			Vertex vertex = {};

			const auto& index = shape.mesh.indices[i];
			vertex.MaterialID = shape.mesh.material_ids[i/3];

			vertex.Position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			if (attrib.texcoords.size()) { // should only be 1 or 2, some bug in debug where there were over 1000 on the spherelines model...
				//m_hasTexCoords = true;
				vertex.TexCoords = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			// Material ID
			//vertex.MaterialID =	shape.mesh.material_ids[attrib.vertices[3 * index.vertex_index + 0]];

			//if (name == "Window")
			//	std::cout << "\nvertex.MaterialID: " << vertex.MaterialID;


			indices.push_back(uniqueVertices[vertex]);

			// THIS MADE A VECTOR SUBSCRIPT OUT OF RANGE ERROR
			//std::cout << "\nmaterialID: " << shape.mesh.material_ids[index.vertex_index];

			//int i = index.

		//	shape.mesh.material_ids[(int)index];
		}

/*		if (name == "Window")
		for (int i = 0; i < shape.mesh.material_ids.size(); i++)
		{
			std::cout << "\nmaterialID: " << shape.mesh.material_ids[i];
		}*/


		//if (m_hasTexCoords) {
			for (int i = 0; i < indices.size(); i += 3) {

				//if (indices[i] > vertices.size()) // to fix weird bug on models with incorrect tex coordinates fucking everything up
				//	break;

				Util::SetNormalsAndTangentsFromVertices(&vertices[indices[i]], &vertices[indices[i + 1]], &vertices[indices[i + 2]]);
			}
		//}
		//std::cout << " indices.size(): " << indices.size() << "\n";

		Mesh* mesh = new Mesh(vertices, indices, shape.name.c_str());
		this->m_meshes.push_back(mesh);




	}

	
	// Create triangle mesh for PhysX
	if (createCollisionMesh) 
	{
		std::vector<PxVec3> m_PxVertices;
		std::vector<PxU32> m_PxIndices;

		for (Mesh* mesh : this->m_meshes) {
			for (auto vertex : mesh->vertices)
				m_PxVertices.push_back(PxVec3(vertex.Position.x, vertex.Position.y, vertex.Position.z));
			for (auto index : mesh->indices)
				m_PxIndices.push_back(index);
		}

		int numVertices = m_PxVertices.size();
		const PxVec3* pxVertices = &m_PxVertices[0];
		int numTriangles = m_PxIndices.size() / 3;
		const PxU32* pxIndices = &m_PxIndices[0];

		m_triMesh = PhysX::p_PhysX->CreateBV33TriangleMesh(numVertices, pxVertices, numTriangles, pxIndices, false, false, false, false, false);
	}



//	m_triMesh = PhysX::p_PhysX->CreateBV33TriangleMesh(numVertices, pxVertices, numTriangles, pxIndices, false, false, false, false, false);



	m_readFromDisk = true;
	m_hasCollisionMesh = createCollisionMesh;
}

void Model::LoadMeshDataToGL()
{
	for (Mesh* mesh : m_meshes)
		mesh->SetupMesh();

	if (m_meshes.size() == 0)
		std::cout << "WARNING: there are 0 meshes\n";

	m_loadedToGL = true;
	//std::cout << "Loaded to GL: " << m_filePath << " (" << m_meshes.size() << " meshes) \n";
}


void Model::Draw(Shader* shader, glm::mat4 modelMatrix, int primitiveType)
{
	for (int i = 0; i < m_meshes.size(); ++i)
		DrawMesh(shader, i, modelMatrix, primitiveType);
}

void Model::DrawMesh(Shader* shader, int meshIndex, glm::mat4 modelMatrix, int primitiveType)
{
	//std::cout << m_filePath <<  " mesh count: " << m_meshes.size() << "\n";

	// If it aint loaded DO NOT TRY DRAW
	if (!m_loadedToGL) return;
	if (m_meshes.size() <= 0) return;

	shader->setMat4("model", modelMatrix);
	m_meshes[meshIndex]->Draw(primitiveType);
}

/*
void Model::Set_Mesh_MaterialID_Set0(const char* meshName, unsigned int materialID)
{
	for (size_t i = 0; i < m_meshes.size(); i++)
	{
		if (Util::StrCmp(meshName, m_meshes[i]->name))
			m_meshes[i]->materialID_Set0 = materialID;
	}
}

void Model::Set_Mesh_MaterialID_Set1(const char* meshName, unsigned int materialID)
{
	for (size_t i = 0; i < m_meshes.size(); i++)
	{
		if (Util::StrCmp(meshName, m_meshes[i]->name))
			m_meshes[i]->materialID_Set1 = materialID;
	}
}
*/
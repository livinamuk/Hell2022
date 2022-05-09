#include "Wall.h"
#include "Helpers/Util.h"

Wall::Wall()
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
}

void Wall::CleanUp()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);

	if (m_rigidStatic)
		m_rigidStatic->release();
}

void Wall::AddVerticesClockwise(Vertex v1, Vertex v2, Vertex v3, Vertex v4)
{
	Util::SetNormalsAndTangentsFromVertices(&v1, &v2, &v3);
	Util::SetNormalsAndTangentsFromVertices(&v3, &v4, &v1);

	v1.Bitangent = glm::vec3(0, 1, 0);
	v2.Bitangent = glm::vec3(0, 1, 0);
	v3.Bitangent = glm::vec3(0, 1, 0);
	v4.Bitangent = glm::vec3(0, 1, 0);
	m_vertices.push_back(v1);
	m_vertices.push_back(v2);
	m_vertices.push_back(v3);
	m_vertices.push_back(v3);
	m_vertices.push_back(v4);
	m_vertices.push_back(v1);
}

void Wall::AddVerticesCounterClockwise(Vertex v1, Vertex v2, Vertex v3, Vertex v4)
{
	Util::SetNormalsAndTangentsFromVertices(&v3, &v2, &v1);
	Util::SetNormalsAndTangentsFromVertices(&v1, &v4, &v3);

	v1.Bitangent = glm::vec3(0, 1, 1);
	v2.Bitangent = glm::vec3(0, 1, 1);
	v3.Bitangent = glm::vec3(0, 1, 1);
	v4.Bitangent = glm::vec3(0, 1, 1);
	m_vertices.push_back(v3);
	m_vertices.push_back(v2);
	m_vertices.push_back(v1);
	m_vertices.push_back(v1);
	m_vertices.push_back(v4);
	m_vertices.push_back(v3);
}

void Wall::BuildMeshFromVertices()
{
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
	glEnableVertexAttribArray(4);

	glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);

	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
	

	std::vector<PxVec3> m_PxVertices;
	std::vector<PxU32> m_PxIndices;

	int i = 0;
	for (auto vertex : m_vertices) {
		m_PxVertices.push_back(PxVec3(vertex.Position.x, vertex.Position.y, vertex.Position.z));
		m_PxIndices.push_back(i);
		i++;
	}
	
	int numVertices = m_PxVertices.size();
	const PxVec3* pxVertices = &m_PxVertices[0];
	int numTriangles = m_PxIndices.size() / 3;
	const PxU32* pxIndices = &m_PxIndices[0];

	m_triMesh = PhysX::p_PhysX->CreateBV33TriangleMesh(numVertices, pxVertices, numTriangles, pxIndices, false, false, false, false, false);


	physx::PxScene* scene = PhysX::GetScene();
	physx::PxPhysics* physics = PhysX::GetPhysics();
	physx::PxMaterial* material = PhysX::GetDefaultMaterial();

	physx::PxTriangleMeshGeometry geom(m_triMesh);

	physx::PxTransform pose(Util::GlmMat4ToPxMat44(Transform().to_mat4()));
	
	m_rigidStatic = PxCreateStatic(*physics, pose, geom, *material);

	scene->addActor(*m_rigidStatic);

	//m_actor->setName("Triangle Mesh");
	//m_actor->userData = new EntityData(PhysicsObjectType::MISC_MESH, this);

	m_rigidStatic->setName("WALL");
	m_rigidStatic->userData = new EntityData(PhysicsObjectType::WALL, this);

	PxShape* shape;
	m_rigidStatic->getShapes(&shape, 1);
	PxFilterData filterData; 
	filterData.word0 = GROUP_RAYCAST;							// enable raycasting
	filterData.word1 = PhysX::CollisionGroup::MISC_OBSTACLE;	// enable collision
	shape->setQueryFilterData(filterData);
	shape->setSimulationFilterData(filterData);
}

void Wall::Draw(Shader* shader)
{
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
}

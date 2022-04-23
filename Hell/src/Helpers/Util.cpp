//#include "hellpch.h"
#include "Util.h"
#include <Windows.h>
#include <random>
#include <iomanip>
#include <sstream>


std::string Util::WeaponStateToString(HUDWeaponAnimationState state)
{
	if (state == HUDWeaponAnimationState::EQUIPPING)
		return "EQUIPPING";
	else if (state == HUDWeaponAnimationState::FIRING)
		return "FIRING";
	else if (state == HUDWeaponAnimationState::HOLSTERING)
		return "HOLSTERING";
	else if (state == HUDWeaponAnimationState::IDLE)
		return "IDLE";
	else if (state == HUDWeaponAnimationState::RELOADING)
		return "RELOADING";
	else if (state == HUDWeaponAnimationState::WALKING)
		return "WALKING";

	return "BUGGG";
}

float Util::YRotationBetweenTwoPoints(glm::vec3 a, glm::vec3 b)
{
	float delta_x = b.x - a.x;
	float delta_y = b.z - a.z;
	float theta_radians = atan2(delta_y, delta_x);
	return -theta_radians;
}

std::string Util::FloatToString(float value, int percision)
{
	std::stringstream stream;
	stream << std::fixed << std::setprecision(percision) << value;
	return stream.str();
}

float Util::FInterpTo(float Current, float Target, float DeltaTime, float InterpSpeed)
{
	// If no interp speed, jump to target value
	if (InterpSpeed <= 0.f)
		return Target;

	// Distance to reach
	const float Dist = Target - Current;

	// If distance is too small, just set the desired location
	if (Dist * Dist < SMALL_NUMBER)
		return Target;

	// Delta Move, Clamp so we do not over shoot.
	const float DeltaMove = Dist * std::clamp(DeltaTime * InterpSpeed, 0.0f, 1.0f);
	return Current + DeltaMove;
}

glm::vec3 Util::Vec3InterpTo(glm::vec3 Current, glm::vec3 Target, float DeltaTime, float InterpSpeed)
{
	float x = FInterpTo(Current.x, Target.x, DeltaTime, InterpSpeed);
	float y = FInterpTo(Current.y, Target.y, DeltaTime, InterpSpeed);
	float z = FInterpTo(Current.z, Target.z, DeltaTime, InterpSpeed);
	return glm::vec3(x, y, z);
}

void Util::DrawUpFacingPlane(Shader* shader, const glm::mat4& modelMatrix)
{
	static unsigned int upFacingPlaneVAO = 0;

	// Setup if you haven't already
	if (upFacingPlaneVAO == 0) {
		Vertex vert0, vert1, vert2, vert3;
		vert0.Position = glm::vec3(-0.5, 0, 0.5);
		vert1.Position = glm::vec3(0.5, 0, 0.5f);
		vert2.Position = glm::vec3(0.5, 0, -0.5);
		vert3.Position = glm::vec3(-0.5, 0, -0.5);
		vert0.TexCoords = glm::vec2(0, 1);
		vert1.TexCoords = glm::vec2(1, 1);
		vert2.TexCoords = glm::vec2(1, 0);
		vert3.TexCoords = glm::vec2(0, 0);
		Util::SetNormalsAndTangentsFromVertices(&vert0, &vert1, &vert2);
		Util::SetNormalsAndTangentsFromVertices(&vert3, &vert0, &vert1);
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		unsigned int i = vertices.size();
		indices.push_back(i);
		indices.push_back(i + 1);
		indices.push_back(i + 2);
		indices.push_back(i + 2);
		indices.push_back(i + 3);
		indices.push_back(i);
		vertices.push_back(vert0);
		vertices.push_back(vert1);
		vertices.push_back(vert2);
		vertices.push_back(vert3);
		unsigned int VBO;
		unsigned int EBO;
		glGenVertexArrays(1, &upFacingPlaneVAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);
		glBindVertexArray(upFacingPlaneVAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
		glEnableVertexAttribArray(5);
		glVertexAttribIPointer(5, 1, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, MaterialID));
	}
	// Draw
	glBindVertexArray(upFacingPlaneVAO);
	shader->setMat4("model", modelMatrix);
	glDrawElements(GL_TRIANGLES, 8, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

std::string Util::CharacterModelAnimationStateToString(CharacterModelAnimationState state)
{
	if (state == CharacterModelAnimationState::STOPPED)
		return "STOPPED";
	//else if (state == CharacterModelAnimationState::WALKING_TO_STOPPED)
	//	return "WALKING_TO_STOPPED";
	else if (state == CharacterModelAnimationState::WALKING)
		return "WALKING";
	else
		return "";
}



bool Util::LineIntersects(glm::vec2 begin_A, glm::vec2 end_A, glm::vec2 begin_B, glm::vec2 end_B, glm::vec2* result)
{
	static const auto SameSign = [](float a, float b) -> bool {
		return ((a * b) >= 0);
	};

	// a
	float x1 = begin_A.x;
	float y1 = begin_A.y;
	float x2 = end_A.x;
	float y2 = end_A.y;

	// b
	float x3 = begin_B.x;
	float y3 = begin_B.y;
	float x4 = end_B.x;
	float y4 = end_B.y;

	float a1, a2, b1, b2, c1, c2;
	float r1, r2, r3, r4;
	float denom, offset, num;

	a1 = y2 - y1;
	b1 = x1 - x2;
	c1 = (x2 * y1) - (x1 * y2);

	r3 = ((a1 * x3) + (b1 * y3) + c1);
	r4 = ((a1 * x4) + (b1 * y4) + c1);

	if ((r3 != 0) && (r4 != 0) && SameSign(r3, r4))
		return DONT_INTERSECT;

	a2 = y4 - y3; // Compute a2, b2, c2
	b2 = x3 - x4;
	c2 = (x4 * y3) - (x3 * y4);
	r1 = (a2 * x1) + (b2 * y1) + c2; // Compute r1 and r2
	r2 = (a2 * x2) + (b2 * y2) + c2;

	if ((r1 != 0) && (r2 != 0) && (SameSign(r1, r2)))
		return DONT_INTERSECT;

	denom = (a1 * b2) - (a2 * b1); //Line segments intersect: compute intersection point.

	if (denom == 0)
		return false;// COLLINEAR;

	// FIND THAT INTERSECTION POINT ALREADY
	{
		// Line AB represented as a1x + b1y = c1
		double a = y2 - y1;
		double b = x1 - x2;
		double c = a * (x1)+b * (y1);
		// Line CD represented as a2x + b2y = c2
		double a1 = y4 - y3;
		double b1 = x3 - x4;
		double c1 = a1 * (x3)+b1 * (y3);
		double det = a * b1 - a1 * b;
		if (det == 0) {

			return DONT_INTERSECT;
		}
		else {
			double x = (b1 * c - b * c1) / det;
			double y = (a * c1 - a1 * c) / det;
			result->x = x;
			result->y = y;
			return DO_INTERSECT;
		}

		return DONT_INTERSECT;
	}
}

bool Util::LineIntersects(glm::vec3 begin_A, glm::vec3 end_A, glm::vec3 begin_B, glm::vec3 end_B, glm::vec3* result)
{
	// wow this is ugly
	glm::vec2 temp;	
	bool i = LineIntersects(glm::vec2(begin_A.x, begin_A.z), glm::vec2(end_A.x, end_A.z), glm::vec2(begin_B.x, begin_B.z), glm::vec2(end_B.x, end_B.z), &temp);
	result->x = temp.x;
	result->y = begin_A.y;
	result->z = temp.y;
	return i;
}

glm::vec3 Util::TranslationFromMat4(glm::mat4& matrix)
{
	float x = matrix[3][0];
	float y = matrix[3][1];
	float z = matrix[3][2];
	return glm::vec3(x, y, z);
}

PxMat44 Util::GlmMat4ToPxMat44(glm::mat4 matrix)
{
	PxMat44 pxMatrix;
	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 4; y++)
			pxMatrix[x][y] = matrix[x][y];
	return pxMatrix;
}

glm::mat4 Util::PxMat44ToGlmMat4(PxMat44 pxMatrix)
{
	glm::mat4 matrix;
	for (int x = 0; x < 4; x++)
		for (int y = 0; y < 4; y++)
			matrix[x][y] = pxMatrix[x][y];
	return matrix;
}

bool Util::GetRayPlaneIntersecion(const glm::vec3& origin, const glm::vec3& dir, const glm::vec3& plane_point, const glm::vec3& normal, float& out)
{
	float d = dot(dir, normal);
	if (d == 0) {
		return false;
	}
	d = dot(plane_point - origin, normal) / d;
	out = d;
	return true;
}


glm::vec3 Util::GetMouseRay(glm::mat4 projectionMatrix, glm::mat4 viewMatrix, int mouseX, int mouseY, int screenWidth, int screenHeight)
{
	float x = (2.0f * mouseX) / screenWidth - 1.0f;
	float y = 1.0f - (2.0f * mouseY) / screenHeight;
	glm::vec3 ray_nds = glm::vec3(x, y, 1.0f);
	glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);
	glm::vec4 ray_eye = inverse(projectionMatrix) * ray_clip;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);
	glm::vec3 ray_wor = glm::normalize(glm::vec3(inverse(viewMatrix) * ray_eye));
	return ray_wor;
}

void Util::PrintVec3(glm::vec3 v)
{
	std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
}

std::string Util::Mat4ToString(glm::mat4& m)
{
	std::stringstream result; 
	result << "(" << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << ")\n";
	result << "(" << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << ", " << m[1][3] << ")\n";
	result << "(" << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << ", " << m[2][3] << ")\n";
	result << "(" << m[3][0] << ", " << m[3][1] << ", " << m[3][2] << ", " << m[3][3] << ")\n";
	return result.str();
}

void Util::PrintMat4(glm::mat4 m)
{
	std::cout << "(" << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << ")\n";
	std::cout << "(" << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << ", " << m[1][3] << ")\n";
	std::cout << "(" << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << ", " << m[2][3] << ")\n";
	std::cout << "(" << m[3][0] << ", " << m[3][1] << ", " << m[3][2] << ", " << m[3][3] << ")\n";
}

std::string Util::FileNameFromPath(std::string filepath)
{
	// Remove directory if present.
	// Do this before extension removal incase directory has a period character.
	const size_t last_slash_idx = filepath.find_last_of("\\/");
	if (std::string::npos != last_slash_idx)
	{
		filepath.erase(0, last_slash_idx + 1);
	}

	// Remove extension if present.
	const size_t period_idx = filepath.rfind('.');
	if (std::string::npos != period_idx)
	{
		filepath.erase(period_idx);
	}
	return filepath;
}
FileType Util::FileTypeFromPath(std::string filepath)
{
	int index = filepath.find_last_of('.') + 1;
	std::string extension = filepath.substr(index);
	//std::cout << "'" << extension << "'\n";

	if (extension == "PNG" || extension == "png")
		return FileType::PNG;
	if (extension == "JPG" || extension == "jpg")
		return FileType::JPG;
	if (extension == "FBX" || extension == "fbx")
		return FileType::FBX;
	if (extension == "OBJ" || extension == "obj")
		return FileType::OBJ;
	else
		return FileType::UNKNOWN;
}

void Util::SetNormalsAndTangentsFromVertices(Vertex* vert0, Vertex* vert1, Vertex* vert2)
{
	//if (!vert0 || !vert1 || !vert2)
	//	return;
	// above can be used to replace the shit below here. its the same.

	// Shortcuts for UVs
	glm::vec3& v0 = vert0->Position;
	glm::vec3& v1 = vert1->Position;
	glm::vec3& v2 = vert2->Position;
	glm::vec2& uv0 = vert0->TexCoords;
	glm::vec2& uv1 = vert1->TexCoords;
	glm::vec2& uv2 = vert2->TexCoords;

	// Edges of the triangle : postion delta. UV delta
	glm::vec3 deltaPos1 = v1 - v0;
	glm::vec3 deltaPos2 = v2 - v0;
	glm::vec2 deltaUV1 = uv1 - uv0;
	glm::vec2 deltaUV2 = uv2 - uv0;

	float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
	glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;
	glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r;

	glm::vec3 normal = Util::NormalFromTriangle(vert0->Position, vert1->Position, vert2->Position);

	vert0->Normal = normal;
	vert1->Normal = normal;
	vert2->Normal = normal;

	vert0->Tangent = tangent;
	vert1->Tangent = tangent;
	vert2->Tangent = tangent;

	vert0->Bitangent = bitangent;
	vert1->Bitangent = bitangent;
	vert2->Bitangent = bitangent;
}

glm::vec3 Util::NormalFromTriangle(glm::vec3 pos0, glm::vec3 pos1, glm::vec3 pos2)
{
	return glm::normalize(glm::cross(pos1 - pos0, pos2 - pos0));
}

std::string Util::BoolToString(bool boolean)
{
	if (boolean)
		return "TRUE";
	else
		return "FALSE";
}

PxMat44 Util::PxMat4FromJSONArray(rapidjson::GenericArray<false, rapidjson::Value> const arr)
{
	PxMat44 m;
	m[0][0] = arr[0].GetFloat();
	m[0][1] = arr[1].GetFloat();
	m[0][2] = arr[2].GetFloat();
	m[0][3] = arr[3].GetFloat();
	m[1][0] = arr[4].GetFloat();
	m[1][1] = arr[5].GetFloat();
	m[1][2] = arr[6].GetFloat();
	m[1][3] = arr[7].GetFloat();
	m[2][0] = arr[8].GetFloat();
	m[2][1] = arr[9].GetFloat();
	m[2][2] = arr[10].GetFloat();
	m[2][3] = arr[11].GetFloat();
	m[3][0] = arr[12].GetFloat();
	m[3][1] = arr[13].GetFloat();
	m[3][2] = arr[14].GetFloat();
	m[3][3] = arr[15].GetFloat();
	return m;
}

glm::mat4 Util::Mat4FromJSONArray(rapidjson::GenericArray<false, rapidjson::Value> const arr)
{
	glm::mat4 m;

	m[0][0] = arr[0].GetFloat();
	m[0][1] = arr[1].GetFloat();
	m[0][2] = arr[2].GetFloat();
	m[0][3] = arr[3].GetFloat();
	m[1][0] = arr[4].GetFloat();
	m[1][1] = arr[5].GetFloat();
	m[1][2] = arr[6].GetFloat();
	m[1][3] = arr[7].GetFloat();
	m[2][0] = arr[8].GetFloat();
	m[2][1] = arr[9].GetFloat();
	m[2][2] = arr[10].GetFloat();
	m[2][3] = arr[11].GetFloat();
	m[3][0] = arr[12].GetFloat();
	m[3][1] = arr[13].GetFloat();
	m[3][2] = arr[14].GetFloat();
	m[3][3] = arr[15].GetFloat();
	/*
	
	m[0][0] = arr[0].GetFloat();
	m[1][1] = arr[1].GetFloat();
	m[2][2] = arr[2].GetFloat();
	m[3][3] = arr[3].GetFloat();
	m[0][0] = arr[4].GetFloat();
	m[1][1] = arr[5].GetFloat();
	m[2][2] = arr[6].GetFloat();
	m[3][3] = arr[7].GetFloat();
	m[0][0] = arr[8].GetFloat();
	m[1][1] = arr[9].GetFloat();
	m[2][2] = arr[10].GetFloat();
	m[3][3] = arr[11].GetFloat();
	m[0][0] = arr[12].GetFloat();
	m[1][1] = arr[13].GetFloat();
	m[2][2] = arr[14].GetFloat();
	m[3][3] = arr[15].GetFloat();
	*/
	return m;
}

glm::vec3 Util::Vec3FromJSONArray(rapidjson::GenericArray<false, rapidjson::Value> const arr)
{
	return glm::vec3(arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat());
}

PxVec3 Util::PxVec3FromJSONArray(rapidjson::GenericArray<false, rapidjson::Value> const arr)
{
	return PxVec3(arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat());
}

std::string Util::PhysicsObjectEnumToString(PhysicsObjectType objectType)
{
	if (objectType == PhysicsObjectType::UNDEFINED)
		return "UNDEFINED";
	if (objectType == PhysicsObjectType::WALL)
		return "WALL";
	if (objectType == PhysicsObjectType::FLOOR)
		return "FLOOR";
	if (objectType == PhysicsObjectType::DOOR)
		return "DOOR";
	if (objectType == PhysicsObjectType::STAIRS)
		return "STAIRS";
	if (objectType == PhysicsObjectType::PLAYER_RAGDOLL)
		return "PLAYER_RAGDOLL";
	if (objectType == PhysicsObjectType::RAGDOLL)
		return "RAGDOLL";
	if (objectType == PhysicsObjectType::CEILING)
		return "CEILING";
	if (objectType == PhysicsObjectType::SHELL_PROJECTILE)
		return "SHELL_PROJECTILE";
	if (objectType == PhysicsObjectType::MISC_MESH)
		return "MISC_MESH";
	if (objectType == PhysicsObjectType::EDITOR_WINDOW)
		return "WINDOW";
	if (objectType == PhysicsObjectType::GLASS)
		return "GLASS";
}

float Util::RandomFloat(float min, float max)
{
	return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}

glm::vec3 Util::GetTranslationFromMatrix(glm::mat4 matrix)
{
	return glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
}

void Util::InterpolateQuaternion(glm::quat& Out, const glm::quat& Start, const glm::quat& End, float pFactor)
{
	// calc cosine theta
	float cosom = Start.x * End.x + Start.y * End.y + Start.z * End.z + Start.w * End.w;

	// adjust signs (if necessary)
	glm::quat end = End;
	if (cosom < static_cast<float>(0.0))
	{
		cosom = -cosom;
		end.x = -end.x;   // Reverse all signs
		end.y = -end.y;
		end.z = -end.z;
		end.w = -end.w;
	}

	// Calculate coefficients
	float sclp, sclq;
	if ((static_cast<float>(1.0) - cosom) > static_cast<float>(0.0001)) // 0.0001 -> some epsillon
	{
		// Standard case (slerp)
		float omega, sinom;
		omega = std::acos(cosom); // extract theta from dot product's cos theta
		sinom = std::sin(omega);
		sclp = std::sin((static_cast<float>(1.0) - pFactor) * omega) / sinom;
		sclq = std::sin(pFactor * omega) / sinom;
	}
	else
	{
		// Very close, do linear interp (because it's faster)
		sclp = static_cast<float>(1.0) - pFactor;
		sclq = pFactor;
	}

	Out.x = sclp * Start.x + sclq * end.x;
	Out.y = sclp * Start.y + sclq * end.y;
	Out.z = sclp * Start.z + sclq * end.z;
	Out.w = sclp * Start.w + sclq * end.w;
}

glm::mat4 Util::Mat4InitScaleTransform(float ScaleX, float ScaleY, float ScaleZ)
{
	/*	glm::mat4 m = glm::mat4(1);
		m[0][0] = ScaleX; m[0][1] = 0.0f;   m[0][2] = 0.0f;   m[0][3] = 0.0f;
		m[1][0] = 0.0f;   m[1][1] = ScaleY; m[1][2] = 0.0f;   m[1][3] = 0.0f;
		m[2][0] = 0.0f;   m[2][1] = 0.0f;   m[2][2] = ScaleZ; m[2][3] = 0.0f;
		m[3][0] = 0.0f;   m[3][1] = 0.0f;   m[3][2] = 0.0f;   m[3][3] = 1.0f;
		return m;*/

	return glm::scale(glm::mat4(1.0), glm::vec3(ScaleX, ScaleY, ScaleZ));
}

glm::mat4 Util::Mat4InitRotateTransform(float RotateX, float RotateY, float RotateZ)
{
	glm::mat4 rx = glm::mat4(1);
	glm::mat4 ry = glm::mat4(1);
	glm::mat4 rz = glm::mat4(1);

	const float x = ToRadian(RotateX);
	const float y = ToRadian(RotateY);
	const float z = ToRadian(RotateZ);

	rx[0][0] = 1.0f; rx[0][1] = 0.0f; rx[0][2] = 0.0f; rx[0][3] = 0.0f;
	rx[1][0] = 0.0f; rx[1][1] = cosf(x); rx[1][2] = -sinf(x); rx[1][3] = 0.0f;
	rx[2][0] = 0.0f; rx[2][1] = sinf(x); rx[2][2] = cosf(x); rx[2][3] = 0.0f;
	rx[3][0] = 0.0f; rx[3][1] = 0.0f; rx[3][2] = 0.0f; rx[3][3] = 1.0f;

	ry[0][0] = cosf(y); ry[0][1] = 0.0f; ry[0][2] = -sinf(y); ry[0][3] = 0.0f;
	ry[1][0] = 0.0f; ry[1][1] = 1.0f; ry[1][2] = 0.0f; ry[1][3] = 0.0f;
	ry[2][0] = sinf(y); ry[2][1] = 0.0f; ry[2][2] = cosf(y); ry[2][3] = 0.0f;
	ry[3][0] = 0.0f; ry[3][1] = 0.0f; ry[3][2] = 0.0f; ry[3][3] = 1.0f;

	rz[0][0] = cosf(z); rz[0][1] = -sinf(z); rz[0][2] = 0.0f; rz[0][3] = 0.0f;
	rz[1][0] = sinf(z); rz[1][1] = cosf(z); rz[1][2] = 0.0f; rz[1][3] = 0.0f;
	rz[2][0] = 0.0f; rz[2][1] = 0.0f; rz[2][2] = 1.0f; rz[2][3] = 0.0f;
	rz[3][0] = 0.0f; rz[3][1] = 0.0f; rz[3][2] = 0.0f; rz[3][3] = 1.0f;

	return rz * ry * rx;
}

glm::mat4 Util::Mat4InitTranslationTransform(float x, float y, float z)
{
	return  glm::translate(glm::mat4(1.0f), glm::vec3(x, y, z));
}

glm::mat4 Util::aiMatrix4x4ToGlm(const aiMatrix4x4& from)
{
	glm::mat4 to;
	//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
	to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
	return to;
}

glm::mat4 Util::aiMatrix3x3ToGlm(const aiMatrix3x3& from)
{
	glm::mat4 to;
	to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = 0.0;
	to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = 0.0;
	to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = 0.0;
	to[0][3] = 0.0; to[1][3] = 0.0; to[2][3] = 0.0; to[3][3] = 1.0;
	return to;
}

bool Util::StrCmp(const char* queryA, const char* queryB)
{
	if (strcmp(queryA, queryB) == 0)
		return true;
	else
		return false;
}

const char* Util::CopyConstChar(const char* text)
{
	char* b = new char[strlen(text) + 1]{};
	std::copy(text, text + strlen(text), b);
	return b;
}
PxQuat Util::PxQuatFromJSONArray(rapidjson::GenericArray<false, rapidjson::Value> const arr)
{
	return PxQuat(arr[0].GetFloat(), arr[1].GetFloat(), arr[2].GetFloat(), arr[3].GetFloat());
}

#include "BloodDecal.h"
#include "Helpers/Util.h" 
#include "Helpers/AssetManager.h" 

BloodDecal::BloodDecal(Transform transform, int type)
{
	m_transform = transform;
	m_type = type;	

	if (m_type != 2) {
		m_localOffset.position.z = 0.55f;
		m_transform.scale = glm::vec3(2.0f);
	}
	else {
		m_localOffset.rotation.y = Util::RandomFloat(0, HELL_PI * 2);;
		m_transform.scale = glm::vec3(1.5f);
	}

	m_modelMatrix = m_transform.to_mat4() * m_localOffset.to_mat4();
}

void BloodDecal::Draw(Shader* shader)
{
	glActiveTexture(GL_TEXTURE2);

	if (m_type == 0)
		glBindTexture(GL_TEXTURE_2D, AssetManager::GetTexturePtr("blood_decal_4")->ID);
	else if (m_type == 1)
		glBindTexture(GL_TEXTURE_2D, AssetManager::GetTexturePtr("blood_decal_6")->ID);
	else if (m_type == 2)
		glBindTexture(GL_TEXTURE_2D, AssetManager::GetTexturePtr("blood_decal_7")->ID);
	else
		glBindTexture(GL_TEXTURE_2D, AssetManager::GetTexturePtr("blood_decal_9")->ID);

	Util::DrawUpFacingPlane(shader, m_modelMatrix);
}

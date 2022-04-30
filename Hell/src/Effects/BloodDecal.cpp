#include "BloodDecal.h"
#include "Helpers/Util.h" 
#include "Helpers/AssetManager.h" 

void BloodDecal::Draw(Shader* shader)
{
	Transform localOffset;

	if (m_type != 2) {
		localOffset.position.z = 0.55f;
		m_transform.scale = glm::vec3(2.0f);
	}
	else {
		localOffset.rotation.y = m_randomRotation;
		m_transform.scale = glm::vec3(1.5f);
	}

	glActiveTexture(GL_TEXTURE2);

	if (m_type == 0)
		glBindTexture(GL_TEXTURE_2D, AssetManager::GetTexturePtr("blood_decal_4")->ID);
	else if (m_type == 1)
		glBindTexture(GL_TEXTURE_2D, AssetManager::GetTexturePtr("blood_decal_6")->ID);
	else if (m_type == 2)
		glBindTexture(GL_TEXTURE_2D, AssetManager::GetTexturePtr("blood_decal_7")->ID);
	else
		glBindTexture(GL_TEXTURE_2D, AssetManager::GetTexturePtr("blood_decal_9")->ID);

	Util::DrawUpFacingPlane(shader,  m_transform.to_mat4() * localOffset.to_mat4());
}

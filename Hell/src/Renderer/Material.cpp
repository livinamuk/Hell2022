#include "Material.h"

void Material::Bind()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ALB);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, NRM);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, RMA);
	//if (E != 0) {
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, E);
	//}
}
void Material::BindToSecondSlot()
{
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, ALB);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, NRM);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, RMA);
}

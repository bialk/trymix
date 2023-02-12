#include <QOpenGLFunctions>
#include <glm/gtx/common.hpp>

/// <summary>
/// Draws a cone into the scene.
/// The default orientation (on identity GL_MODELVIEW matrix) is upright.
/// The default center position (on identity GL_MODELVIEW matrix) is [0.0, 0.0, 0.0].
/// </summary>
/// <param name="fWidthHalf">Specifies the radius in x direction.</param>
/// <param name="fWidthHalf">Specifies the half height in y direction.</param>
/// <param name="fDepthHalf">Specifies the radius in z direction.</param>
/// <param name="dAngleStep">Specifies the angle in radiants, plain surface sections
//                             approximate a curved surface.</param>
/// <param name="pColors">Exactly two colors for cover and bottom or <c>NULL</c>
///                       if no individual colors are to apply.</param>
/// <param name="bTexture">Determine  whether texture coordinates shall be calculated.
/// </param>
/// <remarks>
///       /\                    y (height)
///      |  |      top         /|\
///     /    \                  |
///    |      |                 |      \
///   /_------_\   cover        +-------> x (width)
///  |/        \|              /       /
///  |          |             /
///   \_      _/   bottom   |/_
///     ------              z (depth)
/// </remarks>
void DrawCone(float fWidthHalf, float fHeightHalf, float fDepthHalf,
              double dAngleStep, float const pColors[2][3], bool bTexture = false)
{
    float fReciprocalPrecisition = (float)(dAngleStep / 2*glm::pi<float>());

    // Cone "Cover"
    if (pColors != NULL)
        ::glColor3f(pColors[0][0],pColors[0][1],pColors[0][2]);
    int iSectorCount = 0;
    ::glBegin(GL_TRIANGLES);
    for (double dCoverAngle1 = 0.0F; dCoverAngle1 < 2*glm::pi<float>(); dCoverAngle1 += dAngleStep)
    {
        double dCoverAngle2 = dCoverAngle1 + dAngleStep;
        float  fWidthPart1 = (float)(fWidthHalf * cos(dCoverAngle1));
        float  fWidthPart2 = (float)(fWidthHalf * cos(dCoverAngle2));
        float  fDepthPart1 = (float)(fDepthHalf * sin(dCoverAngle1));
        float  fDepthPart2 = (float)(fDepthHalf * sin(dCoverAngle2));

        // face normal
        ::glNormal3f((fWidthPart1 + fWidthPart2) / 2, 0.0F, (fDepthPart1 + fDepthPart2) / 2);

        // relative texture coordinates
        float fTextureOffsetS1 = iSectorCount       * fReciprocalPrecisition;
        float fTextureOffsetS2 = (iSectorCount + 1) * fReciprocalPrecisition;

        if (bTexture) ::glTexCoord2f(fTextureOffsetS1, 1.0);
        ::glVertex3f(0.0F, fHeightHalf, 0.0F);
        if (bTexture) ::glTexCoord2f(fTextureOffsetS2, 0.0);
        ::glVertex3f(fWidthPart2, -fHeightHalf, fDepthPart2);
        if (bTexture) ::glTexCoord2f(fTextureOffsetS2, 0.0);
        ::glVertex3f(fWidthPart1, -fHeightHalf, fDepthPart1);

        iSectorCount++;
    }
    ::glEnd();

    // Cone Bottom
    if (pColors != NULL)
      ::glColor3f(pColors[1][0],pColors[1][1],pColors[1][2]);
    iSectorCount = 0;
    ::glBegin(GL_POLYGON); // ? TRIANGLE_FAN ?
    for (double dBottomAngle = 0; dBottomAngle < 2*glm::pi<float>(); dBottomAngle += dAngleStep)
    {
        // face normal
        ::glNormal3f(0.0F, -1.0F, 0.0F);

        double c = cos(dBottomAngle);
        double s = sin(dBottomAngle);

        // relative texture coordinates
        float fTextureOffsetS = 0.5F + (float)(0.5F * c);
        float fTextureOffsetT = 0.5F + (float)(0.5F * s);

        if (bTexture) ::glTexCoord2f(fTextureOffsetS, fTextureOffsetT);
       ::glVertex3f((float)(fWidthHalf * c), -fHeightHalf, (float)(fDepthHalf * s));
    }
    ::glEnd();
}

void drawCube(){

  glBegin(GL_QUADS);                // Begin drawing the color cube with 6 quads
    // Top face (y = 1.0f)
    // Define vertices in counter-clockwise (CCW) order with normal pointing out
    glColor3f(0.0f, 1.0f, 0.0f);     // Green
    glNormal3f(0,1,0);
    glVertex3f( 1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f,  1.0f);
    glVertex3f( 1.0f, 1.0f,  1.0f);

    // Bottom face (y = -1.0f)
    glNormal3f(0,-1,0);
    glColor3f(1.0f, 0.5f, 0.0f);     // Orange
    glVertex3f( 1.0f, -1.0f,  1.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);

    // Front face  (z = 1.0f)
    glNormal3f(0,0,1);
    glColor3f(1.0f, 0.0f, 0.0f);     // Red
    glVertex3f( 1.0f,  1.0f, 1.0f);
    glVertex3f(-1.0f,  1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f( 1.0f, -1.0f, 1.0f);

    // Back face (z = -1.0f)
    glNormal3f(0,0,-1);
    glColor3f(1.0f, 1.0f, 0.0f);     // Yellow
    glVertex3f( 1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);

    // Right face (x = 1.0f)
    glNormal3f(1,0,0);
    glColor3f(1.0f, 0.0f, 1.0f);     // Magenta
    glVertex3f(1.0f,  1.0f, -1.0f);
    glVertex3f(1.0f,  1.0f,  1.0f);
    glVertex3f(1.0f, -1.0f,  1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);

    // Left face (x = -1.0f)
    glNormal3f(-1,0,0);
    glColor3f(0.0f, 0.0f, 1.0f);     // Blue
    glVertex3f(-1.0f,  1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);

  glEnd();  // End of drawing color-cube
}


void drawTestScene(){
  // draw test scene
  if(true){
    glPushAttrib(GL_ENABLE_BIT|GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);

    glPushMatrix();
    glTranslatef(-100.,-100.,-100.);
    glScalef(30,30,30);
    //glDisable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK,GL_DIFFUSE);
    drawCube();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.,0.,-30);
    glScalef(30,30,30);
    //glDisable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK,GL_DIFFUSE);
    drawCube();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.,0.,-100.);
    glScalef(20,20,20);
    {
      float a[][3] = {{1.,0.,0.},{0.,1.,1.}};
      DrawCone(1, 3, 2, 2*glm::pi<float>()/100, a );
    }
    glPopMatrix();

    glPushMatrix();
    glTranslatef(100.,100.,0.);
    glScalef(30,30,30);
    {
      float a[][3] = {{1.,0.,0.},{0.,1.,1.}};
      DrawCone(1, 3, 2, 2*glm::pi<float>()/100, a );
    }
    glPopMatrix();

    glPopAttrib();
  }
}

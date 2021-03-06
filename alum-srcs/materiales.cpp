// *********************************************************************
// **
// ** Gestión de materiales y texturas (implementación)
// **
// ** Copyright (C) 2014 Carlos Ureña
// **
// ** This program is free software: you can redistribute it and/or modify
// ** it under the terms of the GNU General Public License as published by
// ** the Free Software Foundation, either version 3 of the License, or
// ** (at your option) any later version.
// **
// ** This program is distributed in the hope that it will be useful,
// ** but WITHOUT ANY WARRANTY; without even the implied warranty of
// ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// ** GNU General Public License for more details.
// **
// ** You should have received a copy of the GNU General Public License
// ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
// **
// *********************************************************************


#include "matrices-tr.hpp"
#include "materiales.hpp"

using namespace std ;

const bool trazam = false ;

//*********************************************************************

PilaMateriales::PilaMateriales()
{
   actual = nullptr ;
}
// -----------------------------------------------------------------------------

void PilaMateriales::activarMaterial( Material * material )
{
   if ( material != actual )
   {
      actual = material ;
      if ( actual != nullptr )
         actual->activar();
   }
}
// -----------------------------------------------------------------------------

void PilaMateriales::activarActual()
{
   if ( actual != nullptr )
      actual->activar() ;
}
// -----------------------------------------------------------------------------

void PilaMateriales::push(  )
{
   pila.push_back( actual );
}
// -----------------------------------------------------------------------------

void PilaMateriales::pop(  )
{
   if ( pila.size() == 0 )
   {
      cout << "error: intento de hacer 'pop' de un material en una pila de materiales vacía." << endl << flush ;
      exit(1);
   }

   Material * anterior = pila[pila.size()-1] ;
   pila.pop_back();
   activarMaterial( anterior );  // cambia 'actual'
}

//**********************************************************************

Textura::Textura( const std::string & nombreArchivoJPG )
{
   imagen = new jpg::Imagen(nombreArchivoJPG);
   enviada = false;
   modo_gen_ct = mgct_desactivada;
}

// ---------------------------------------------------------------------

//----------------------------------------------------------------------

void Textura::enviar()
{
  assert( imagen != NULL );
  glGenTextures( 1, &ident_textura );
  glBindTexture( GL_TEXTURE_2D, ident_textura );
  unsigned int ancho = imagen->tamX();
  unsigned int alto = imagen->tamY();
  unsigned char * texels = imagen->leerPixels();
  gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB, ancho, alto, GL_RGB, GL_UNSIGNED_BYTE, texels);
  enviada = true;
}

//----------------------------------------------------------------------

Textura::~Textura( )
{
   using namespace std ;
   cout << "destruyendo textura...imagen ==" << imagen << endl ;
   if ( imagen != NULL )
      delete imagen ;

   imagen = NULL ;
   cout << "hecho (no hecho!)" << endl << flush ;
}

//----------------------------------------------------------------------
// por ahora, se asume la unidad de texturas #0

void Textura::activar(  )
{
  glEnable( GL_TEXTURE_2D );
  if (!enviada)
    enviar();
  else
    glBindTexture( GL_TEXTURE_2D, ident_textura );

  if (modo_gen_ct == mgct_desactivada) {
    glDisable( GL_TEXTURE_GEN_S );
    glDisable( GL_TEXTURE_GEN_T );
  } else {
    glEnable( GL_TEXTURE_GEN_S );
    glEnable( GL_TEXTURE_GEN_T );
    if (modo_gen_ct == mgct_coords_objeto) {
      glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
      glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR );
      glTexGenfv( GL_S, GL_OBJECT_PLANE, coefs_s );
      glTexGenfv( GL_T, GL_OBJECT_PLANE, coefs_t );
    } else if (modo_gen_ct == mgct_coords_ojo) {
      glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR );
      glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR );
      glTexGenfv( GL_S, GL_EYE_PLANE, coefs_s );
      glTexGenfv( GL_T, GL_EYE_PLANE, coefs_t );
    }
  }
}


// *********************************************************************

TexturaXY::TexturaXY( const string & nom ) : Textura(nom)
{
  modo_gen_ct = mgct_coords_objeto;
  coefs_s[1] = 1.0;
  coefs_s[0] = coefs_s[2] = coefs_s[3] = 0.0;
  coefs_t[0] = 1.0;
  coefs_s[1] = coefs_s[2] = coefs_s[3] = 0.0;
}


// *********************************************************************

Material::Material()
{
   iluminacion = false ;
   tex = NULL ;
   coloresCero() ;
}
// ---------------------------------------------------------------------

Material::Material( const std::string & nombreArchivoJPG )
{
   iluminacion    = true ;
   tex            = new Textura( nombreArchivoJPG ) ;

   coloresCero();

   del.emision   = VectorRGB(0.0,0.0,0.0,1.0);
   del.ambiente  = VectorRGB( 0.0, 0.0, 0.0, 1.0);
   del.difusa    = VectorRGB( 0.5, 0.5, 0.5, 1.0 );
   del.especular = VectorRGB( 1.0, 1.0, 1.0, 1.0 );

   del.emision   = VectorRGB(0.0,0.0,0.0,1.0);
   del.ambiente  = VectorRGB( 0.0, 0.0, 0.0, 1.0);
   tra.difusa    = VectorRGB( 0.2, 0.2, 0.2, 1.0 );
   tra.especular = VectorRGB( 0.2, 0.2, 0.2, 1.0 );

}

// ---------------------------------------------------------------------
// crea un material usando textura y coeficientes: ka,kd,ks y exponente
// (la textura puede ser NULL, la ilum. queda activada)

Material::Material( Textura * text, float ka, float kd, float ks, float exp )
:  Material()
{
   tex = text;
   iluminacion = true;
   coloresCero();

   del.ambiente = tra.ambiente = VectorRGB(ka, ka, ka, 1.0);
   del.difusa = tra.difusa = VectorRGB(kd, kd, kd, 1.0);
   del.especular = tra.especular = VectorRGB(ks, ks, ks, 1.0);
   del.exp_brillo = tra.exp_brillo = exp;

   ponerNombre("material con textura e iluminación") ;

 }

// ---------------------------------------------------------------------
// crea un material con un color único para las componentes ambiental y difusa
// en el lugar de textura (textura == NULL)
Material::Material( const Tupla3f & colorAmbDif, float ka, float kd, float ks, float exp )
{
   tex = NULL;
   iluminacion = true;
   coloresCero();

   del.ambiente = tra.ambiente = VectorRGB(ka * colorAmbDif(0),
      ka * colorAmbDif(1), ka * colorAmbDif(2), 1.0);
   del.ambiente = tra.ambiente = VectorRGB(ka * colorAmbDif(0),
     ka * colorAmbDif(1), ka * colorAmbDif(2), 1.0);
   del.especular = tra.especular = VectorRGB(ks, ks, ks, 1.0);
   del.exp_brillo = tra.exp_brillo = exp;

   // TODO: Falta aqui?

   ponerNombre("material color plano, ilum.") ;
}
// ---------------------------------------------------------------------

Material::Material( const float r, const float g, const float b )
{
  tex = NULL;
  iluminacion = false,
  coloresCero();

  color = VectorRGB(r, g, b, 1.0);
  ponerNombre("material color plano, no ilum.");
}

//----------------------------------------------------------------------

void Material::coloresCero()
{
   const VectorRGB ceroRGBopaco(0.0,0.0,0.0,1.0);

   color         =

   del.emision   =
   del.ambiente  =
   del.difusa    =
   del.especular =

   tra.emision   =
   tra.ambiente  =
   tra.difusa    =
   tra.especular = ceroRGBopaco ;

   del.exp_brillo =
   tra.exp_brillo = 1.0 ;
}
//----------------------------------------------------------------------

Material::~Material()
{
   if ( tex != nullptr )
   {  delete tex ;
      tex = nullptr ;
   }
}
//----------------------------------------------------------------------

void Material::ponerNombre( const std::string & nuevo_nombre )
{
   nombre_mat = nuevo_nombre ;
}
//----------------------------------------------------------------------

std::string Material::nombre() const
{
   return nombre_mat ;
}
//----------------------------------------------------------------------

void Material::activar(  )
{
  if (iluminacion) {
    glEnable( GL_LIGHTING );
    glMaterialfv( GL_FRONT, GL_EMISSION, del.emision );
    glMaterialfv( GL_FRONT, GL_AMBIENT, del.ambiente );
    glMaterialfv( GL_FRONT, GL_DIFFUSE, del.difusa );
    glMaterialfv( GL_FRONT, GL_SPECULAR, del.especular );
    glMaterialf( GL_FRONT, GL_SHININESS, del.exp_brillo );
    glMaterialfv( GL_BACK, GL_EMISSION, tra.emision );
    glMaterialfv( GL_BACK, GL_AMBIENT, tra.ambiente );
    glMaterialfv( GL_BACK, GL_DIFFUSE, tra.difusa );
    glMaterialfv( GL_BACK, GL_SPECULAR, tra.especular );
    glMaterialf( GL_BACK, GL_SHININESS, tra.exp_brillo );
  } else {
    glDisable( GL_LIGHTING );
    glColor4fv( color );
    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glColorMaterial( GL_FRONT_AND_BACK, GL_EMISSION );
    glColorMaterial( GL_FRONT_AND_BACK, GL_SPECULAR );
  }

  if (tex != NULL)
    tex->activar();
  else
    glDisable( GL_TEXTURE_2D );


}

//**********************************************************************

FuenteLuz::FuenteLuz( GLfloat p_longi_ini, GLfloat p_lati_ini, const VectorRGB & p_color )
{
   //CError();

   if ( trazam )
      cout << "creando fuente de luz." <<  endl << flush ;

   // inicializar parámetros de la fuente de luz
   longi_ini = p_longi_ini ;
   lati_ini  = p_lati_ini  ;
   longi = longi_ini ;
   lati  = lati_ini ;

   col_ambiente  = p_color ;
   col_difuso    = p_color ;
   col_especular = p_color ;

   esDireccional = true;

   ind_fuente = -1 ; // la marca como no activable hasta que no se le asigne indice

   //CError();
}

//----------------------------------------------------------------------

FuenteLuz::FuenteLuz( const Tupla3f& posicion, const VectorRGB & p_color )
{
   //CError();

   if ( trazam )
      cout << "creando fuente de luz." <<  endl << flush ;

   // inicializar parámetros de la fuente de luz
   pos = posicion;

   col_ambiente  = p_color ;
   col_difuso    = p_color ;
   col_especular = p_color ;

   esDireccional = false;

   ind_fuente = -1 ; // la marca como no activable hasta que no se le asigne indice

   //CError();
}

//----------------------------------------------------------------------

void FuenteLuz::activar()
{
  assert( ind_fuente >= 0 && ind_fuente < 8 );
  glEnable( GL_LIGHT0 + ind_fuente );
  // Configuramos los colores
  glLightfv (GL_LIGHT0 + ind_fuente, GL_AMBIENT, col_ambiente);
  glLightfv (GL_LIGHT0 + ind_fuente, GL_DIFFUSE, col_difuso);
  glLightfv (GL_LIGHT0 + ind_fuente, GL_SPECULAR, col_especular);

  if (esDireccional) {
    const GLfloat ejeZ[4] = {0.0, 0.0, 1.0, 0.0};
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();
    glRotatef( longi, 0.0, 1.0, 0.0 );
    glRotatef( lati, -1.0, 0.0, 0.0);
    glLightfv( GL_LIGHT0 + ind_fuente, GL_POSITION, ejeZ );
    glPopMatrix();
  } else {
    const GLfloat posf[4] = { pos(0), pos(1), pos(2), 1.0 };
    glLightfv( GL_LIGHT0 + ind_fuente, GL_POSITION, posf );
  }
}

//----------------------------------------------------------------------

bool FuenteLuz::gestionarEventoTeclaEspecial( int key )
{
   bool actualizar = true ;
   const float incr = 3.0f ;

   if (esDireccional) {
     switch( key )
     {
        case GLFW_KEY_RIGHT:
           longi = longi+incr ;
           break ;
        case GLFW_KEY_LEFT:
           longi = longi-incr ;
           break ;
        case GLFW_KEY_UP:
           lati = std::min( lati+incr, 90.0f) ;
           break ;
        case GLFW_KEY_DOWN:
           lati = std::max( lati-incr, -90.0f ) ;
           break ;
        case GLFW_KEY_HOME:
           lati  = lati_ini ;
           longi = longi_ini ;
           break ;
        default :
           actualizar = false ;
           cout << "tecla no usable para la fuente de luz." << endl << flush ;
     }
   } else
    actualizar = false;

   //if ( actualizar )
   //   cout << "fuente de luz cambiada: longi == " << longi << ", lati == " << lati << endl << flush ;
   return actualizar ;
}

//----------------------------------------------------------------------

bool FuenteLuz::esDir() {
  return esDireccional;
}

float FuenteLuz::getAngulo(unsigned int angulo) {
  assert(esDireccional);
  return (angulo == 1 ? lati : longi);
}

void FuenteLuz::variarAngulo(unsigned angulo, float incremento)
{
  assert(esDireccional);
  switch (angulo) {
    case 0:
      longi = longi + incremento;
      if (longi > 360.0)
        longi -= 360;
      else if (longi < -360.0)
        longi += 360;
      break;
    case 1:
      lati = lati + incremento;
      if (lati > 90.0)
        lati = 90.0;
      else if (lati < -90.0)
        lati = -90.0;
      break;
  }
}

//**********************************************************************

ColFuentesLuz::ColFuentesLuz()
{
   max_num_fuentes = 8 ;
}
//--------------------------------------d --------------------------------

void ColFuentesLuz::insertar( FuenteLuz * pf )  // inserta una nueva
{
   assert( pf != nullptr );

   pf->ind_fuente = vpf.size() ;
   vpf.push_back( pf ) ;
}
//----------------------------------------------------------------------
// activa una colección de fuentes de luz

void ColFuentesLuz::activar( bool esProyectiva )
{
  glEnable( GL_LIGHTING );
  if (esProyectiva)
    glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE );
  else
    glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
  glDisable( GL_COLOR_MATERIAL );
  glLightModeli( GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR );
  for (unsigned int i = 0; i < vpf.size(); ++i)
    vpf[i]->activar();
  for (unsigned int i = vpf.size(); i < max_num_fuentes; ++i)
    glDisable(GL_LIGHT0 + i);
}

//----------------------------------------------------------------------
FuenteLuz * ColFuentesLuz::ptrFuente( unsigned i )
{
   assert(i < vpf.size()) ;
   return vpf[i] ;
}


//----------------------------------------------------------------------
ColFuentesLuz::~ColFuentesLuz()
{
   for( unsigned i = 0 ; i < vpf.size() ; i++ )
   {
      assert( vpf[i] != NULL );
      delete vpf[i] ;
      vpf[i] = NULL ;
   }
}

//----------------------------------------------------------------------

unsigned ColFuentesLuz::size() {
  return vpf.size();
}

//**********************************************************************

FuenteDireccional::FuenteDireccional( float alpha_inicial, float beta_inicial )
: FuenteLuz(alpha_inicial, beta_inicial, Tupla4f(0.9, 0.9, 0.9, 1.0)) {}

//**********************************************************************

FuentePosicional::FuentePosicional( const Tupla3f& posicion )
  : FuenteLuz(posicion, Tupla4f(1.0, 1.0, 1.0, 1.0)) {}

//**********************************************************************

ColeccionFuentesP4::ColeccionFuentesP4() {
  insertar(new FuenteDireccional(0, 0));
  //insertar(new FuentePosicional(Tupla3f(1.0, 1.0, 0.0)));
}

//**********************************************************************

MaterialLata::MaterialLata() : Material("../imgs/lata-coke.jpg")
{
  coloresCero();
  del.ambiente = tra.ambiente = VectorRGB(0.3, 0.3, 0.3, 1.0);
  del.difusa = tra.difusa = VectorRGB(0.7, 0.7, 0.7, 1.0);
  del.especular = tra.especular = VectorRGB(0.7, 0.7, 0.7, 1.0);
  del.exp_brillo = tra.exp_brillo = 5.0;
}

//**********************************************************************

constexpr float colorPlata = 169.0 / 255.0;

MaterialTapasLata::MaterialTapasLata()
  : Material(Tupla3f(colorPlata, colorPlata, colorPlata), 0.8, 1.0, 0.6, 8.0) {}

//**********************************************************************


MaterialPeonMadera::MaterialPeonMadera() :
 Material(new TexturaXY("../imgs/text-madera.jpg"), 0.5, 0.75, 0.5, 5.0  )
{
}

//**********************************************************************

MaterialPeonBlanco::MaterialPeonBlanco() : Material(Tupla3f(1.0, 1.0, 1.0), 0.65, 0.8, 0.6, 5.0)
{
//  del.ambiental = tra.ambiental = VectorRGB(1.0, 1.0, 1.0, 1.0);
}

//**********************************************************************

MaterialPeonNegro::MaterialPeonNegro() : Material(Tupla3f(0.01, 0.01, 0.01), 0.6, 0.5, 0.4, 8.0) {}

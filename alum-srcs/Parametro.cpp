#include <cassert>
#define _USE_MATH_DEFINES
#include <cmath>
#include "Parametro.hpp"
#include "Objeto3D.hpp"

// -----------------------------------------------------------------------------

// crear grado de libertad no acotado

Parametro::Parametro(
   const std::string &  p_descripcion,
   Matriz4f *           p_ptr_mat,
   TFuncionCMF          p_fun_calculo_matriz,
   bool                 p_acotado,
   float                p_c,
   float                p_s,
   float                p_f
) : descripcion(p_descripcion), acotado(p_acotado),
    fun_calculo_matriz(p_fun_calculo_matriz), c(p_c), s(p_s), f(p_f),
    ptr_mat(p_ptr_mat)
{
  valor_norm = 0;
  velocidad = v0;
}
// -----------------------------------------------------------------------------

void Parametro::siguiente_cuadro()
{
  valor_norm += velocidad;
  *ptr_mat = fun_calculo_matriz(leer_valor_actual());
}
// -----------------------------------------------------------------------------

void Parametro::reset()
{
  valor_norm = 0;
  velocidad = v0;
  *ptr_mat = fun_calculo_matriz(leer_valor_actual());
}
// -----------------------------------------------------------------------------
void Parametro::incrementar()
{
  valor_norm += incremento;
  *ptr_mat = fun_calculo_matriz(leer_valor_actual());
}
// -----------------------------------------------------------------------------

void Parametro::decrementar()
{
    valor_norm -= incremento;
    *ptr_mat = fun_calculo_matriz(leer_valor_actual());
}
// -----------------------------------------------------------------------------
void Parametro::acelerar()
{
   velocidad += a;
}
// -----------------------------------------------------------------------------
void Parametro::decelerar()
{
  velocidad -= a;
  if (velocidad < 0)
    velocidad = 0;
}
// -----------------------------------------------------------------------------

float Parametro::leer_valor_actual()
{
  if (acotado)
    return c + s * sin(f* 2 * M_PI * valor_norm);
  return c + s * valor_norm;
}

// -----------------------------------------------------------------------------
float Parametro::leer_velocidad_actual()
{
   return velocidad;

}
// -----------------------------------------------------------------------------

std::string Parametro::leer_descripcion()
{
   return descripcion;

}
// -----------------------------------------------------------------------------

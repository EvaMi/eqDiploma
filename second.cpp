//seq-распределение
#include <seq/seq.h>
//gl.h-Includes OpenGL and GLEW headers. EQ_GL_CALL(code)
#include <eq/gl.h>
#include <lunchbox/file.h>

#include <stdlib.h>
#include <time.h>
#include <cmath>

#include <eqHello/fragmentShader.glsl.h>
#include <eqHello/vertexShader.glsl.h>

namespace second
{
//Класс рендерера
class Renderer : public seq::Renderer
{
public:
    Renderer( seq::Application& application )
        : seq::Renderer( application )
        , _vertexArray( 0 )
        , _vertexBuffer( 0 )
        , _indexBuffer( 0 )
        , _colorBuffer( 0 )
        , _program( 0 )
        , _matrixUniform( 0 )
    {}
    virtual ~Renderer() {}

protected:
    void draw( co::Object* frameData ) final;
    bool initContext( co::Object* initData ) final;
    bool exitContext() final;

private:
    bool _loadShaders();
    void _setupCube(int param);
    int chain(int j, int i);

    typedef vmml::vector< 3, GLushort > Vector3s;

    std::vector< seq::Vector3f > _vertices;
    std::vector< Vector3s > _triangles;
    std::vector< seq::Vector3f > _colors;
    //Переменные
    GLuint _vertexArray;
    GLuint _vertexBuffer;
    GLuint _indexBuffer;
    GLuint _colorBuffer;
    GLuint _program;
    GLuint _matrixUniform;
};
//Класс приложения
class Application : public seq::Application
{
    //Спецификатор virtual создаёт виртуальную функцию. (полиморфизм)
    //Виртуальная функция — это член базового класса, который может быть переопределён производным классом.
    //Если функция не переопределяется производным классом, используется определение базового класса.
    virtual ~Application() {}
public:
    //инициализация рендерера
    virtual seq::Renderer* createRenderer() { return new Renderer( *this ); }
    //инициализация приложения
    bool init( int argc, char** argv, co::Object* initData ) final
    {
        return seq::Application::init( argc, argv, initData );
    }
};
//понять, что такое ланчбокс
//The lunchbox library provides C++ classes to abstract the underlying
//operating system and to implement common helper functionality for multi-
//threaded applications. Examples are lunchbox::Clock providing a high-resolution
//timer, or lunchbox::MTQueue providing a thread-safe, blocking FIFO.
typedef lunchbox::RefPtr< Application > ApplicationPtr;
}
//главная функция
int main( const int argc, char** argv )
{
    second::ApplicationPtr app = new second::Application;
    if( app->init( argc, argv, 0 ) && app->run( 0 ) && app->exit( ))
        return EXIT_SUCCESS;

    return EXIT_FAILURE;
}

int second::Renderer::chain(int j, int i){
        int imax=4;
        return (i+j*(imax+1));
}
//Задание фигуры
void second::Renderer::_setupCube(int param)
{
    if( _vertexArray )
    {
      return;
    }
    //размеры сетки
    int imax=48;
    int jmax=48;
    //размер индексного массива
    //int sizeindex;
    //начальная координата x
    float x0=-1;
    //начальная координата z
    float z0=-1;
    //шаг сетки по оси x
    float dx=0.04;
    //шаг сетки по оси z
    float dz=0.04;
    // массив для хранения координаты x
    float x[100];
    // массив для хранения координаты y
    float y[100][100];
    //массив для хранения координаты z
    float z[100];
    //массив для хранения координат вершин для записи в буфер
    //float vertex[];

    //x=new float [imax+1];
    //z=new float [jmax+1];
    //y=new float [jmax+1][imax+1];
    //vertex=new float[(jmax+1)*(imax+1)*3];


    //заполним массивы x и z координатами сетки
    for (int i=0; i<=imax; i++){
             x[i]=x0+i*dx;
    }
    for (int j=0; j<=jmax; j++){
             z[j]=z0+j*dz;
    }
    //clock_t start;
    //start=clock();
    //double time=(double) (start)/CLOCKS_PER_SEC;
    // заполним массив Y значениями функции
    for (int i=0; i<=imax; i++){
             for (int j=0; j<=imax; j++){
                      y[i][j]=exp(param*(x[i]*x[i]+z[j]*z[j]));
                      //y[j][i]=cos(0.005*time+5*(z[j]+x[i]));
             }
    }

    std::cout << y[0][0] << std::endl;
    //вершины
    // заполним массив координат vertex
    for (int i=0; i<imax-1; i++){
             for (int j=0; j<imax-1; j++){
                      _vertices.push_back(seq::Vector3f(x[i],z[j],y[j][i]));
                      _vertices.push_back(seq::Vector3f(x[i],z[j+1],y[j+1][i]));
                      _vertices.push_back(seq::Vector3f(x[i+1],z[j],y[j][i+1]));
                      _vertices.push_back(seq::Vector3f(x[i+1],z[j],y[j][i+1]));
                      _vertices.push_back(seq::Vector3f(x[i+1],z[j+1],y[j+1][i+1]));
                      _vertices.push_back(seq::Vector3f(x[i],z[j+1],y[j+1][i]));
             }
    }

    //грани
    // вспомогательная функция
    // возвращает порядковый номер вершины по известным j и i
    int j=0;
    while (j<(100000-2))
    {
      _triangles.push_back(Vector3s(j,j+1,j+2));
      j=j+3;
    }
    /*
    while (j < jmax) {
             // лента слева направо
             for (int i = 0; i <= imax; i++) {
                      _triangles.push_back(Vector3s(chain(i,j),chain(j,i),chain(j+1,i)));
             }
             if (j < jmax-1){
                      // вставим хвостовой индекс для связки
                      chain(j+1,imax);
             }
             // переводим ряд
             j++;

             // проверяем достижение конца
             if (j < jmax){
                      // лента справа налево
                      for (int i = imax; i >= 0; i--) {
                               _triangles.push_back(Vector3s(chain(i,j),chain(j,i),chain(j+1,i)));
                      }
                      if (j < jmax-1){
                               // вставим хвостовой индекс для связки
                               chain(j+1,0);
                      }
                      // переводим ряд
                      j++;
             }
    }*/

    //цвета
    j=0;
    while (j<(1000000-2))
    {
      _colors.push_back(seq::Vector3f(0.5, 0.5, 0.5));
      _colors.push_back(seq::Vector3f(0.5, 0.5, 1.0));
      _colors.push_back(seq::Vector3f(0.6, 1.0, 0.5));
      j++;
    }
    /*
    _colors = {
        seq::Vector3f(0.5, 0.5, 0.5),
        seq::Vector3f(0.5, 0.5, 1.0),
        seq::Vector3f(0.6, 1.0, 0.5),
        seq::Vector3f(0.6, 1.0, 1.0),
        seq::Vector3f(0.6, 1.0, 0.5),
        seq::Vector3f(0.6, 1.0, 0.5),
        seq::Vector3f(0.6, 1.0, 0.5),
        seq::Vector3f(0.6, 1.0, 0.5),
        seq::Vector3f(0.6, 1.0, 0.5),
    };*/
    //Выяснить, что делает данная строчка
    seq::ObjectManager& om = getObjectManager();
    //Активируем объект vertex array object
    _vertexArray = om.newVertexArray( &_vertexArray );
    //Подключаем vertex array object с настойками полигональной модели
    EQ_GL_CALL( glBindVertexArray( _vertexArray ));

    _vertexBuffer = om.newBuffer( &_vertexBuffer );
    //Активируем буфер с вершинными атрибутами
    EQ_GL_CALL( glBindBuffer( GL_ARRAY_BUFFER, _vertexBuffer ));
    //Выделение памяти на видеокарте, копирование массива из оперативной памяти. Цель буфера, размер, указатель, предполагаемый способ использования буфера
    EQ_GL_CALL( glBufferData( GL_ARRAY_BUFFER,
                              _vertices.size() * sizeof(seq::Vector3f),
                              _vertices.data(), GL_STATIC_DRAW ));
    //Устанавливаем настройки: 0й атрибут, 3 компоненты типа GL_FLOAT, не нужно нормализовать, 0 - значения расположены в массиве впритык, 0 - сдвиг от начала
    //В команде glVertexAttribPointer указывается:
    //-номер атрибута, для которого задаются настройки
    //-количество компонентов (1, 2, 3, 4)
    //-тип данных компонентов (GL_FLOAT и другие)
    //-нужно ли нормализовать значения, т.е. приводить к диапазону [0; 1]
    //-расстояние в байтах между атрибутами соседних вершин - нужно, если атрибуты чередуются
    //-сдвиг от начала буфера (особенность этой функции - нужно приводить к void*)
    EQ_GL_CALL( glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 ));

    //Буфер цветов
    _colorBuffer = om.newBuffer( &_colorBuffer );
    EQ_GL_CALL( glBindBuffer( GL_ARRAY_BUFFER, _colorBuffer ));
    EQ_GL_CALL( glBufferData( GL_ARRAY_BUFFER,
                              _colors.size() * sizeof(seq::Vector3f),
                              _colors.data(), GL_STATIC_DRAW ));
    //Устанавливаем настройки: 1й атрибут, 3 компоненты типа GL_FLOAT, не нужно нормализовать, 0 - значения расположены в массиве впритык, 0 - сдвиг от начала массива
    EQ_GL_CALL( glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 ));

    _indexBuffer = om.newBuffer( &_indexBuffer );
    //Подключаем буфер с индексами.
    //Вершины в буфере располагаются в произвольном порядке, но добавляется индексный буфер, в котором хранятся номера вершин, которые образуют треугольник.
    //Например, индексный буфер { 0, 3, 4, 3, 1, 5 }.
    //Здесь 2 треугольника, которые образованы вершинами с номерами (0, 3, 4) и (3, 1, 5).
    EQ_GL_CALL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _indexBuffer ));
    //Цель буфера штвуч-GL_ELEMENT_ARRAY_BUFFER
    EQ_GL_CALL( glBufferData( GL_ELEMENT_ARRAY_BUFFER,
                              _triangles.size() * sizeof(Vector3s),
                              _triangles.data(), GL_STATIC_DRAW ));

    EQ_GL_CALL( glBindBuffer( GL_ARRAY_BUFFER, 0 ));
    EQ_GL_CALL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ));
    //Отключаем vertex array object
    EQ_GL_CALL( glBindVertexArray( 0 ));
}


//Загрузка шейдеров
bool second::Renderer::_loadShaders()
{
    //vertexShader_glsl
    //Изначально координаты вершин меша заданы в произвольной локальной системе координат.
    //Чтобы отобразить 3D-модель на экране монитора, необходимо преобразовать все вершины модели в экранную систему координат

    //fragmentShader_glsl
    //Для каждого фрагмента вызывается фрагментный шейдер, на вход которому передаются интерполированные значения из вершинного шейдера.

    //Что делает данная строчка?
    //Returns the object manager of this renderer.
    //Object Manager The object manager is, strictly speaking, not a part of the window.
    //It is mentioned here since the eqPly window uses an object manager.
    //The state object in eqPly gathers all rendering state, which includes an object manager for OpenGL object allocation.
    //The object manager (OM) is a utility class and can be used to manage OpenGL objects across shared contexts.
    //Typically one OM is used for each set of shared contexts of a single GPU.
    seq::ObjectManager& om = getObjectManager();

    if( _program )
        return true;

    _program = om.newProgram( &_program );
    if( !seq::linkProgram( om.glewGetContext(), _program, vertexShader_glsl,
                           fragmentShader_glsl ))
    {
        return false;
    }

    EQ_GL_CALL( glUseProgram( _program ));
    _matrixUniform = glGetUniformLocation( _program, "MVP" );
    EQ_GL_CALL( glUseProgram( 0));
    return true;
}

//Инициализация контекста
bool second::Renderer::initContext( co::Object* initData )
{
    if( !seq::Renderer::initContext( initData ))
        return false;

    if( !_loadShaders( ))
        return false;

    _setupCube(-10);
    return true;
}

//Выход из контекста, удаление буферов
bool second::Renderer::exitContext()
{
    seq::ObjectManager& om = getObjectManager();
    om.deleteProgram( &_program );
    om.deleteBuffer( &_vertexBuffer );
    om.deleteBuffer( &_colorBuffer );
    om.deleteBuffer( &_indexBuffer );
    om.deleteVertexArray( &_vertexArray );

    return seq::Renderer::exitContext();
}

bool surface_create()
{
  typedef vmml::vector< 3, GLushort > Vector3s;
  std::vector< seq::Vector3f > _vertices;
  std::vector< Vector3s > _triangles;
  //размеры сетки
  int imax=48;
  int jmax=48;
  //размер индексного массива
  //int sizeindex;
  //начальная координата x
  float x0=-1;
  //начальная координата z
  float z0=-1;
  //шаг сетки по оси x
  float dx=0.04;
  //шаг сетки по оси z
  float dz=0.04;
  // массив для хранения координаты x
  float x[49];
  // массив для хранения координаты y
  float y[49][49];
  //массив для хранения координаты z
  float z[49];
  //массив для хранения координат вершин для записи в буфер
  //float vertex[];

  //x=new float [imax+1];
  //z=new float [jmax+1];
  //y=new float [jmax+1][imax+1];
  //vertex=new float[(jmax+1)*(imax+1)*3];


  //заполним массивы x и z координатами сетки
  for (int i=0; i<=imax; i++){
           x[i]=x0+i*dx;
  }
  for (int j=0; j<=jmax; j++){
           z[j]=z0+j*dz;
  }
  // заполним массив Y значениями функции
  for (int j=0; j<=jmax; j++){
           for (int i=0; i<=imax; i++){
                    y[j][i]=(-3*(x[i]*x[i]+z[j]*z[j]));
           }
  }
  // заполним массив координат vertex
  for (int j=0; j<=jmax; j++){
           for (int i=0; i<=imax; i++){
                    _vertices.push_back(seq::Vector3f(x[i],y[j][i],z[j]));
           }
  }

  //грани
  // ,,,9, 9,  9,14,8,13,7,12,6,11,5,10, 10,  10,15,11,16,12,17,13,18,14,19
  _triangles = {
      // front
      //Vector3s(0, 1, 2),
      Vector3s(0,5,1),
      Vector3s(6,2,7),
      Vector3s(3,8,4),
      // top
      /*
      Vector3s(3, 2, 6),
      Vector3s(6, 7, 3),
      // back
      Vector3s(7, 6, 5),
      Vector3s(5, 4, 7),
      // right
      Vector3s(1, 5, 6),
      Vector3s(6, 2, 1)
      */
  };
  return true;
}
//Отрисовка
/** The rendering routine, a.k.a., glutDisplayFunc() */
void second::Renderer::draw( co::Object* /*frameData*/ )
{
    //Для запуска отрисовки меша нужно подключить соответствующий ему vertex array object и вызвать команду отрисовки.
    //Применить текущие параметры рендеринга в OpenGL.
    //Этот метод устанавливает буфер, wdtn, экрана, а также проекции и вид матрицы.
    applyRenderContext(); // set up OpenGL State
    //Что такое Matrix4f?
    //Трансформационная матрица, отвечает за поворот фигуры
    const seq::Matrix4f mvp = getFrustum().computePerspectiveMatrix() *
                              getViewMatrix() * getModelMatrix();

    EQ_GL_CALL( glUseProgram( _program ));
    //Передаем наши трансформации в текущий шейдер
    // Для каждой модели, которую вы выводите MVP будет различным (как минимум часть M)
    //glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
    EQ_GL_CALL( glUniformMatrix4fv( _matrixUniform, 1, GL_FALSE, mvp.data( )));
    //Подключаем vertex array object с настойками полигональной модели

    EQ_GL_CALL( glBindVertexArray( _vertexArray ));
    //Включаем 0й вершинный атрибут - координаты
    EQ_GL_CALL( glEnableVertexAttribArray( 0 ));
    //EQ_GL_CALL(glEnable(GL_CULL_FACE));
    //EQ_GL_CALL(glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST));
    //Включаем 1й вершинный атрибут - цвета
    EQ_GL_CALL( glEnableVertexAttribArray( 1 ));
    EQ_GL_CALL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, _indexBuffer ));
    //Семейство команд glDrawArrays предназначено для безиндексных мешей, семейство glDrawElements - для индексных мешей.
    //Геометрический примитив, количество индексов в буфере с индексами, тип данных в буфере с индексами, сдвиг от начала буфера с индексами
    EQ_GL_CALL( glDrawElements( GL_TRIANGLES, GLsizei(_triangles.size() * 3),
                                GL_UNSIGNED_SHORT, 0 ));

    EQ_GL_CALL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 ));
    EQ_GL_CALL( glDisableVertexAttribArray( 1 ));
    EQ_GL_CALL( glDisableVertexAttribArray( 0 ));
    //Отключаем vertex array object
    EQ_GL_CALL( glBindVertexArray( 0 ));
    EQ_GL_CALL( glUseProgram( 0 ));
}

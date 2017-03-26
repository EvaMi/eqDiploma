#include <co/buffer.h>
#include <co/connection.h>
#include <co/connectionDescription.h>
#include <co/connectionSet.h>
#include <co/init.h>

#include <lunchbox/clock.h>
#include <lunchbox/monitor.h>
#include <lunchbox/file.h>
#include <lunchbox/test.h>


#include <seq/seq.h>
//gl.h-Includes OpenGL and GLEW headers. EQ_GL_CALL(code)
#include <eq/gl.h>

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <cmath>

#include <eqHello/fragmentShader.glsl.h>
#include <eqHello/vertexShader.glsl.h>

//переменная определения окончания дочернего потока чтения
lunchbox::Monitor< bool > s_done( false );

namespace second
{
  // массив для хранения координаты x
  float xm[20];
  // массив для хранения координаты y
  float ym[20][20];
  //массив для хранения координаты z
  float zm[20];
class Reader : public lunchbox::Thread
{
public:
    explicit Reader( co::ConnectionPtr connection ) : connection_( connection )
        { TEST( start( )); }

    void run() override
    {
        co::ConnectionPtr listener = connection_;
        connection_ = listener->acceptSync();
        TESTINFO( connection_, listener->getDescription( ));

        co::Buffer buffer;
        co::BufferPtr syncBuffer;
        buffer.reserve(sizeof(float)*23*sizeof(float)*20);
        std::cout<<  "Max buffer size="  << std::endl;
        std::cout<<  buffer.getMaxSize	()  << std::endl;
        float (&sequence)[23][20] = *reinterpret_cast< float(*)[23][20] >( buffer.getData( ));

        s_done = false;

        connection_->recvNB( &buffer,  sizeof(float)*23*sizeof(float)*20);
        TEST( connection_->recvSync( syncBuffer ));
        TEST( syncBuffer == &buffer );
        TEST( buffer.getSize() ==  sizeof(float)*23*sizeof(float)*20);
        std::cout<<  "Test buffer!!!"  << std::endl;
        std::cout<<  sequence[3][1]<< std::endl;
        //TEST( sequence == ++i || sequence == 0xdeadbeef );
        buffer.setSize( 0 );
        std::cout<<  "Test buffer2!!!"  << std::endl;

        s_done = true;
        std::cout<<  "Test buffer3!!!"  << std::endl;
        connection_->recvNB( &buffer,  sizeof(float)*23*sizeof(float)*20);
        std::cout<<  "Test buffer4!!!"  << std::endl;
        TEST( !connection_->recvSync( syncBuffer ));
        std::cout<<  "Test buffer5!!!"  << std::endl;
        std::cout<<  "Test buffer!!!"  << std::endl;
        std::cout<<  sequence[3][1]<< std::endl;

        //размеры сетки
        int imax=20;
        int jmax=20;

        for (int i=0; i<=imax; i++){
                 xm[i]=sequence[1][i];
        }
        for (int j=0; j<=jmax; j++){
                 zm[j]=sequence[2][j];
        }
        // заполним массив Y значениями функции
        for (int i=3; i<=imax; i++){
                 for (int j=0; j<=imax; j++){
                          ym[i][j]=sequence[i][j];
                 }
        }
        #ifndef _WIN32
          TEST( connection_->isClosed( ));
        #endif
          connection_ = 0;
    }

private:
    co::ConnectionPtr connection_;
};
//инициализация connection
bool _initialize( co::ConnectionDescriptionPtr desc,
                  co::ConnectionPtr& listener,
                  co::ConnectionPtr& writer )
{
    desc->setHostname( "127.0.0.1" );
    desc->port=49868;

    listener = co::Connection::create( desc );
    if( !listener )
    {
        std::cout << ": not supported" << std::endl;
        return false;
    }
    const bool listening = listener->listen();
    TESTINFO( listening, desc );
    listener->acceptNB();
    desc->setHostname( "127.0.0.1" );
    //desc->port=52822;
    //co::LocalNodePtr server = new co::LocalNode;

    writer = co::Connection::create( desc );
    TEST( writer );
    return true;
}

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
    void _setupCube();

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

typedef lunchbox::RefPtr< Application > ApplicationPtr;
typedef lunchbox::RefPtr<co::LocalNode> LocalNodePtr;
typedef lunchbox::RefPtr<co::Node> NodePtr;
typedef lunchbox::RefPtr<co::Buffer> BufferPtr;
typedef lunchbox::RefPtr<co::Connection>ConnectionPtr;
typedef lunchbox::RefPtr<co::ConnectionDescription>ConnectionDescriptionPtr;
}

int main( int argc, char **argv )
{
    //TEST(( PACKETSIZE % 8 ) == 0 );
    co::init( argc, argv );

    co::ConnectionDescriptionPtr desc = new co::ConnectionDescription;
    desc->type = co::CONNECTIONTYPE_TCPIP;;

    co::ConnectionPtr writer;
    co::ConnectionPtr listener;
    if( !second::_initialize( desc, listener, writer ))
          std::cout<< "Dont inicialize" << std::endl;

    second::Reader readThread( listener );
    TEST( writer->connect( ));

    //размеры сетки
    int imax=20;
    int jmax=20;
    //размер индексного массива
    //int sizeindex;
    //начальная координата x
    float x0=-1;
    //начальная координата z
    float z0=-1;
    //шаг сетки по оси x
    float dx=0.08;
    //шаг сетки по оси z
    float dz=0.08;
    // массив для хранения координаты x
    //float x[49];
    // массив для хранения координаты y
    float y[23][20];//imax+3!!!
    //массив для хранения координаты z
    //float z[49];
    //массив для хранения всех координат вершин для записи в буфер
    //float common[51][49];
    y[0][0]=20;
    y[0][1]=20;

    //заполним массивы x и z координатами сетки
    for (int i=0; i<=imax; i++){
             y[1][i]=x0+i*dx;
    }
    for (int j=0; j<=jmax; j++){
             y[2][j]=z0+j*dz;
    }
    // заполним массив Y значениями функции
    for (int i=3; i<=imax; i++){
             for (int j=0; j<=imax; j++){
                      y[i][j]=exp(-4*(y[1][j]*y[2][j]+y[2][j]*y[2][j]));
             }
    }

    std::cout<<  "Test send!!!"  << std::endl;
    std::cout<<  y[3][1] << std::endl;
    std::cout<<  "Test send!!!"  << std::endl;
    TEST( writer->send( &y, (sizeof(float)*23*sizeof(float)*20)));

    s_done.waitEQ( true );
    writer->close();
    readThread.join();
    listener->close();

    std::cout<<  "Check"<< std::endl;
    std::cout<<  second::ym[3][1]<< std::endl;
    //прорисовка
    //from main second.cpp
    second::ApplicationPtr app = new second::Application;
    if( app->init( argc, argv, 0 ) && app->run( 0 ) && app->exit( ))
        return EXIT_SUCCESS;

    co::exit();
    return EXIT_SUCCESS;
}

//Задание фигуры
void second::Renderer::_setupCube()
{
    //размеры сетки
    int imax=20;
    int jmax=20;
    //Забрать данные из Reader
    std::cout<<  "Check 2"<< std::endl;
    std::cout << second::ym[0][0] << std::endl;
    //вершины
    // заполним массив координат vertex
    for (int i=0; i<imax-1; i++){
             for (int j=0; j<jmax-1; j++){
                      _vertices.push_back(seq::Vector3f(second::xm[i],second::zm[j],second::ym[j][i]));
                      _vertices.push_back(seq::Vector3f(second::xm[i],second::zm[j+1],second::ym[j+1][i]));
                      _vertices.push_back(seq::Vector3f(second::xm[i+1],second::zm[j],second::ym[j][i+1]));
                      _vertices.push_back(seq::Vector3f(second::xm[i+1],second::zm[j],second::ym[j][i+1]));
                      _vertices.push_back(seq::Vector3f(second::xm[i+1],second::zm[j+1],second::ym[j+1][i+1]));
                      _vertices.push_back(seq::Vector3f(second::xm[i],second::zm[j+1],second::ym[j+1][i]));
             }
    }

    int j=0;
    while (j<(100000-2))
    {
      _triangles.push_back(Vector3s(j,j+1,j+2));
      j=j+3;
    }

    //цвета
    j=0;
    while (j<(1000000-2))
    {
      _colors.push_back(seq::Vector3f(0.5, 0.5, 0.5));
      _colors.push_back(seq::Vector3f(0.5, 0.5, 1.0));
      _colors.push_back(seq::Vector3f(0.6, 1.0, 0.5));
      j++;
    }
  
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

    _setupCube();
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

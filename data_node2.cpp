//seq-распределение
#include <seq/seq.h>
//gl.h-Includes OpenGL and GLEW headers. EQ_GL_CALL(code)
#include <eq/gl.h>
#include <co/co.h>
#include <lunchbox/file.h>

#include <stdlib.h>
#include <stdio.h>
#include <cmath>

#include <eqHello/fragmentShader.glsl.h>
#include <eqHello/vertexShader.glsl.h>

namespace data_node
{
    class LocalNode : public co::LocalNode
    {
      //public:
      //virtual  ~LocalNode(){}
    };
    class Node : public co::Node
    {
      //public:
      //virtual  ~Node(){}
    };
    class Connection :  public co::Connection
    {
      //virtual  ~Connection(){}
    };
    class ConnectionDescription : public co::ConnectionDescription
    {
      //public:
      //virtual  ~ConnectionDescription(){}
    };
float create_surface()
{
  /*
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
  */
  return 0;
}
typedef lunchbox::RefPtr<co::LocalNode> LocalNodePtr;
typedef lunchbox::RefPtr<co::Node> NodePtr;
typedef lunchbox::RefPtr<co::Connection>ConnectionPtr;
typedef lunchbox::RefPtr<co::ConnectionDescription>ConnectionDescriptionPtr;
}


int main(const int argc, char** argv)
{
  //pointers
  co::init(argc, argv);
  co::ConnectionDescriptionPtr connDesc=new co::ConnectionDescription;
  co::ConnectionDescriptionPtr connDesc1=new co::ConnectionDescription;

  //Local node for proxy
  co::LocalNodePtr server = new co::LocalNode;
  connDesc1->setHostname( "localhost" );
  connDesc->port=49868;
  connDesc->setFilename("pfiles_");
  server->addConnectionDescription( connDesc );
  server->listen( );

  //proxy node
  co::NodePtr serverProxy = new co::Node;
  serverProxy->addConnectionDescription( connDesc );

  //connection description for main node
  connDesc1 = new co::ConnectionDescription;
  connDesc1->type = co::CONNECTIONTYPE_TCPIP;
  connDesc1->setHostname( "localhost" );
  connDesc1->port=52822;
  connDesc1->setFilename("mfiles_");

  //Main local node
  co::LocalNodePtr client = new co::LocalNode;
  client->addConnectionDescription( connDesc1 );
  client->listen( );
  std::cout<< "wow:" << std::endl;
  client->connect( serverProxy );
  client->connect( serverProxy );
  std::cout<< "wowowowow:" << std::endl;

  co::ConnectionPtr con1;
  con1=serverProxy->co::Node::getConnection	(false);
  //con1=co::Connection::create(connDesc1);
  //con1->co::Connection::listen();
  std::cout <<con1->co::Connection::getDescription	(		)<< std::endl;
  int intValue = static_cast<int>(con1->co::Connection::getState());
  std::cout <<intValue<< std::endl;
  con1->co::Connection::unlockSend();
  /*
  std::cout<< "nononono:" << std::endl;
  con1->co::Connection::send("gndi",2,false);
  std::cout<< "niuhiuhiuhuifh:" << std::endl;
  */
  //configure buffer
  co::Buffer buffer;
  co::BufferPtr syncBuffer;
  buffer.reserve( 12 );
  co::Buffer buffer1;
  buffer1.reserve( 12 );

  std::cout<< "nononono:" << std::endl;
  con1->co::Connection::send(&buffer,12,false );

  //con1->co::Connection::readSync(buffer,12);
  std::cout<< "niuhiuhiuhuifh:" << std::endl;
  //clear end close
  std::cout<< "End:" << std::endl;
  client->disconnect( serverProxy);
  client->close( );
  server->close( );

  serverProxy = 0;
  client = 0;
  server = 0;

  co::exit();

  return EXIT_SUCCESS;

}

//seq-распределение
#include <seq/seq.h>
//gl.h-Includes OpenGL and GLEW headers. EQ_GL_CALL(code)
#include <eq/gl.h>
#include <co/co.h>
#include <lunchbox/file.h>

#include <stdlib.h>
#include <stdio.h>
#include <cstdlib> // Для работы с функцией system()

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
  co::ConnectionPtr con1;

  connDesc = new co::ConnectionDescription;
  connDesc->type = co::CONNECTIONTYPE_TCPIP;
  connDesc->setHostname( "localhost" );
  connDesc->port=49868;

  co::LocalNodePtr client = new co::LocalNode;
  client->addConnectionDescription( connDesc );
  client->listen( );

  co::NodePtr serverProxy = new co::Node;
  serverProxy->addConnectionDescription( connDesc );

  //тут надо сделать command handle!!!
  //client->connect( serverProxy );
  //client->co::LocalNode::dispatchCommand(Null);

  //getchar();
  std::cout<< "End:" << std::endl;
  //client->disconnect( serverProxy);
  client->close( );

  serverProxy = 0;
  client = 0;

  co::exit();
  getchar();
  return EXIT_SUCCESS;

}

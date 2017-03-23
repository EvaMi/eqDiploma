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

#define PACKETSIZE (123456)

namespace data_node
{

  class Reader : public lunchbox::Thread
  {
  public:
    explicit Reader( co::ConnectionPtr connection ): connection_( connection ){}
      void run() override
      {
          co::ConnectionPtr listener = connection_;
          connection_ = listener->acceptSync();

          co::Buffer buffer;
          co::BufferPtr syncBuffer;
          buffer.reserve( PACKETSIZE );
          uint64_t& sequence = *reinterpret_cast< uint64_t* >( buffer.getData( ));
          sequence = 0;
          //uint64_t i = 0;


          while( sequence != 0xdeadbeef )
          {
              connection_->recvNB( &buffer, 123456 );
              sequence = 0xdeadbeef;
              buffer.setSize( 0 );
          }

          connection_->recvNB( &buffer, 123456 );
          connection_ = 0;
          std::cout<<"buffer" << std::endl;
          std::cout<<buffer.getData( ) << std::endl;
      }
    private:
      co::ConnectionPtr connection_;
  };

typedef lunchbox::RefPtr<co::LocalNode> LocalNodePtr;
typedef lunchbox::RefPtr<co::Node> NodePtr;
typedef lunchbox::RefPtr<co::Buffer> BufferPtr;
typedef lunchbox::RefPtr<co::Connection>ConnectionPtr;
typedef lunchbox::RefPtr<co::ConnectionDescription>ConnectionDescriptionPtr;
}


int main(const int argc, char** argv)
{
  //pointers
  co::init(argc, argv);
  data_node::ConnectionDescriptionPtr connDesc=new co::ConnectionDescription;
  co::ConnectionPtr con1;

  connDesc = new co::ConnectionDescription;
  connDesc->type = co::CONNECTIONTYPE_TCPIP;
  connDesc->setHostname( "127.0.0.1" );
  connDesc->setFilename("cfiles_");
  connDesc->port=49868;

  data_node::LocalNodePtr client = new co::LocalNode;
  client->addConnectionDescription( connDesc );
  //client->listen( );
  data_node::ConnectionPtr listener;
  listener = co::Connection::create( connDesc );
  listener->listen();
  listener->acceptNB();

  //Local node for proxy
  data_node::LocalNodePtr server = new co::LocalNode;
  connDesc = new co::ConnectionDescription;
  connDesc->setHostname( "127.0.0.1" );
  connDesc->port=52822;
  connDesc->setFilename("sfiles_");
  server->addConnectionDescription( connDesc );
  server->listen( );

  data_node::NodePtr serverProxy = new co::Node;
  serverProxy->addConnectionDescription( connDesc );
  std::cout<< "1111gldehndkhnd:" << std::endl;
  //if(client->connect( serverProxy ))
    //std::cout<< "Succesfull connected" << std::endl;

    data_node::ConnectionPtr writer;
    writer = co::Connection::create( connDesc );

  data_node::Reader readThread( listener );
  writer->connect( );

  //connection=serverProxy->co::Node::getConnection();
  //std::cout<<connection->co::Connection::getDescription()<< std::endl;
  float x0=-1;
  float x[100];
  float dx=0.04;
  int imax=48;
  for (int i=0; i<=imax; i++){
           x[i]=x0+i*dx;
  }
  //uint64_t sequence = 1;
  //++sequence;
  //for (int i=0; i<=imax; i++){
  //        writer->send( &x[i], sizeof(float));
  //}
  if(writer->send( &x, sizeof(float)*100))
    std::cout<< "aloe:" << std::endl;

  uint64_t sequence = 0;
  sequence= 0xdeadbeef;
  if(writer->send( &sequence, sizeof( uint64_t )))
      std::cout<< "aloe:" << std::endl;

  std::cout<< "gldkjhdljhd:" << std::endl;
  std::cout<< "End:" << std::endl;

  //client->disconnect( serverProxy);
  client->close( );
  server->close( );

  serverProxy = 0;
  client = 0;
  server=0;

  co::exit();
  return EXIT_SUCCESS;

}

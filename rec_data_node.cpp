#include <lunchbox/test.h>
#include <co/buffer.h>
#include <co/connection.h>
#include <co/connectionDescription.h>
#include <co/connectionSet.h>
#include <co/init.h>

#include <lunchbox/clock.h>
#include <lunchbox/monitor.h>
#include <iostream>

#define PACKETSIZE (123456)
#define RUNTIME (1000) // ms

lunchbox::Monitor< bool > s_done( false );

namespace
{
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
        buffer.reserve(  sizeof(float)*2 );
        float& sequence = *reinterpret_cast< float* >( buffer.getData( ));
        sequence = 0;
        //uint64_t i = 0;

        s_done = false;

        while( sequence != 1010101010 )
        {
            connection_->recvNB( &buffer,  sizeof(float) );
            TEST( connection_->recvSync( syncBuffer ));
            TEST( syncBuffer == &buffer );
            TEST( buffer.getSize() ==  sizeof(float) );
            std::cout<<  "Test buffer!!!"  << std::endl;
            std::cout<<  sequence  << std::endl;
            //TEST( sequence == ++i || sequence == 0xdeadbeef );
            buffer.setSize( 0 );
        }

        s_done = true;
        connection_->recvNB( &buffer,  sizeof(float) );
        TEST( !connection_->recvSync( syncBuffer ));
#ifndef _WIN32
        TEST( connection_->isClosed( ));
#endif
        connection_ = 0;
    }

private:
    co::ConnectionPtr connection_;
};
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
typedef lunchbox::RefPtr<co::LocalNode> LocalNodePtr;
typedef lunchbox::RefPtr<co::Node> NodePtr;
typedef lunchbox::RefPtr<co::Buffer> BufferPtr;
typedef lunchbox::RefPtr<co::Connection>ConnectionPtr;
typedef lunchbox::RefPtr<co::ConnectionDescription>ConnectionDescriptionPtr;
}

int main( int argc, char **argv )
{
    TEST(( PACKETSIZE % 8 ) == 0 );
    co::init( argc, argv );

    co::ConnectionDescriptionPtr desc = new co::ConnectionDescription;
    desc->type = co::CONNECTIONTYPE_TCPIP;;

    co::ConnectionPtr writer;
    co::ConnectionPtr listener;
    if( !_initialize( desc, listener, writer ))
          std::cout<< "Dont inicialize" << std::endl;

    Reader readThread( listener );
    TEST( writer->connect( ));

    int imax=48;
    float x0=-1;
    float dx=0.04;
    float x[49];

    for (int i=0; i<=imax; i++){
             x[i]=x0+i*dx;
    }
    std::cout<<  "Test send!!!"  << std::endl;
    std::cout<<  x[1]  << std::endl;
    std::cout<<  "Test send!!!"  << std::endl;
    float out[1];
    //lunchbox::Clock clock;
    //uint64_t sequence = 0;

    //while( clock.getTime64() < RUNTIME )
    //{
      //out[0] = ++sequence;
      TEST( writer->send( &x[1], sizeof(float) ));
    //}

    out[0] = 1010101010;
    TEST( writer->send( out,  sizeof(float) ));

    s_done.waitEQ( true );
    writer->close();
    readThread.join();
    listener->close();

    co::exit();
    return EXIT_SUCCESS;
}

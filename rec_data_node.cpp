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
        buffer.reserve(sizeof(float)*23*sizeof(float)*20);
        std::cout<<  "Max buffer size="  << std::endl;
        std::cout<<  buffer.getMaxSize	()  << std::endl;
        float (&sequence)[23][20] = *reinterpret_cast< float(*)[23][20] >( buffer.getData( ));
        //sequence = 0;
        //uint64_t i = 0;

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
        //float* xm[49];
        //xm=*syncBuffer;
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
    float dx=0.04;
    //шаг сетки по оси z
    float dz=0.04;
    // массив для хранения координаты x
    //float x[49];
    // массив для хранения координаты y
    float y[23][20];//imax+3!!!
    //массив для хранения координаты z
    //float z[49];
    //массив для хранения всех координат вершин для записи в буфер
    //float common[51][49];
    y[0][0]=48;
    y[0][1]=48;

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
                      //y[j][i]=cos(0.005*time+5*(z[j]+x[i]));
             }
    }
    //x[imax+1]=1010101010;
    std::cout<<  "Test send!!!"  << std::endl;
    std::cout<<  y[3][1] << std::endl;
    //std::cout<<  &y << std::endl;
    std::cout<<  "Test send!!!"  << std::endl;
    //float out[1];
    //lunchbox::Clock clock;
    //uint64_t sequence = 0;

    //while( clock.getTime64() < RUNTIME )
    //{
      //out[0] = ++sequence;
    TEST( writer->send( &y, (sizeof(float)*23*sizeof(float)*20)));
    //}

    //out[0] = 1010101010;
    //TEST( writer->send( out,  sizeof(float) ));

    s_done.waitEQ( true );
    writer->close();
    readThread.join();
    listener->close();

    co::exit();
    return EXIT_SUCCESS;
}

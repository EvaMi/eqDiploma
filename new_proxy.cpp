#include <co/connection.h>
#include <co/connectionDescription.h>
#include <co/connectionSet.h>
#include <co/global.h>
#include <co/init.h>

#include <iostream>

#define BUFFERSIZE 1024

int main( int argc, char **argv )
{
    if( argc < 3 )
    {
        std::cout << "Usage: " << argv[0] << " input output" << std::endl;
        return EXIT_FAILURE;
    }

    co::init( argc, argv );

    co::ConnectionDescriptionPtr listen = new co::ConnectionDescription;
    listen->type = co::CONNECTIONTYPE_TCPIP;
    connDesc->setHostname( "localhost" );
    listen->port=49868;
    //std::string listenArg( argv[1] );
    //listen->fromString( listenArg );

    co::ConnectionDescriptionPtr forward = new co::ConnectionDescription;
    forward->type = co::CONNECTIONTYPE_TCPIP;
    connDesc1->setHostname( "localhost" );
    connDesc->port=52822;
    //forward->port = co::Global::getDefaultPort() + 1;
    //std::string forwardArg( argv[2] );
    //forward->fromString( forwardArg );

    // wait for input connection
    co::ConnectionPtr connection = co::Connection::create( listen );
    if( !connection )
    {
        std::cout << "Unsupported connection: " << listen << std::endl;
        co::exit();
        return EXIT_FAILURE;
    }

    if( !connection->listen( ))
    {
        std::cout << "Can't open listening socket " << listen << std::endl;
        co::exit();
        return EXIT_FAILURE;
    }
    connection->acceptNB();

    co::ConnectionSet connections;
    connections.addConnection( connection );
    connections.select();

    // remove listener, add input connection
    connection = connections.getConnection();
    connections.removeConnection( connection );

    co::ConnectionPtr input = connection->acceptSync();
    uint8_t inputBuffer[ BUFFERSIZE ];
    input->readNB( inputBuffer, BUFFERSIZE );
    connections.addConnection( input );

    // connect forwarding socket
    co::ConnectionPtr output = co::Connection::create( forward );
    if( !output->connect( ))
    {
        std::cout << "Can't connect forwarding socket " << forward << std::endl;
        co::exit();
        return EXIT_FAILURE;
    }

    uint8_t outputBuffer[ BUFFERSIZE ];
    output->readNB( outputBuffer, BUFFERSIZE );
    connections.addConnection( output );

    while( true )
    {
        switch( connections.select( )) // ...get next request
        {
            case co::ConnectionSet::EVENT_DATA:  // new data
            {
                connection = connections.getConnection();
                const bool isInput = (connection == input);
                uint8_t* buffer = isInput ? inputBuffer : outputBuffer;
                const int64_t read = connection->readSync( buffer, BUFFERSIZE,true );
                if( read < 0 ) // error
                {
                    std::cout << "Socket disconnected" << std::endl;
                    return EXIT_SUCCESS;
                }
                else if( read )
                {
                    if( isInput )
                        output->send( buffer, read );
                    else
                        input->send( buffer, read );
                }
                connection_->readNB( buffer, BUFFERSIZE);
                break;
            }

            case co::ConnectionSet::EVENT_DISCONNECT:
            case co::ConnectionSet::EVENT_INVALID_HANDLE:
                std::cout << "Socket disconnected" << std::endl;
                return EXIT_SUCCESS;

            case co::ConnectionSet::EVENT_INTERRUPT:
                break;

            case co::ConnectionSet::EVENT_CONNECT:
            default:
                assert( 0 );
                break;
        }
    }

    return EXIT_SUCCESS;
}

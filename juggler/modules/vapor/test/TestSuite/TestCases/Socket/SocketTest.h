#ifndef _SOCKET_TEST_H
#define _SOCKET_TEST_H

#include <stdio.h>
#include <string.h>
#include <iostream>

#include <TestCase.h>
#include <TestSuite.h>
#include <TestCaller.h>

#include <IO/Socket/Socket.h>
#include <IO/Socket/SocketStream.h>
#include <IO/Socket/InetAddr.h>
#include <include/System.h>

#include <Threads/Thread.h>
#include <Threads/ThreadFunctor.h>
#include <Sync/Mutex.h>

#include <vector>

namespace {

struct _thread_args {
	vpr::SocketStream* mSock;
};


   
   
typedef struct _thread_args thread_args_t;

}

namespace vprTest
{

class SocketTest : public TestCase
{
public:
   SocketTest( std::string name ) : TestCase (name), mThreadAssertTest(true), testSendRecv_buffer( "we're sending DATA!!!" )
   {
      mOpenServerSuccess=0;
      mNumSServer=2;
      mNumClient=2;
      mFinishFlag=false;
      mBlockingFlag=true;
      mStartFlag=false;
      mReadnFlag=false;
   }
   
   // use this within your threads (CppUnit doesn't catch the assertTest there)
   // then test mThreadAssertTest with assertTest in your parent func.
   // then reset it to true.
   /*
   #define threadAssertTest( testcase )          \
   {                                          \
      if (testcase == false)                 \
      {                                        \
         mThreadAssertTest = false;           \
      }                                      \
      assertTest( testcase );                   \
   }
   */

   void threadAssertTest( bool testcase, std::string text=std::string("none") )
   {                                         
      if (testcase == false)
      {
         mThreadAssertTest = false;
         std::cerr << "threadAssertTest - failed: " << text << std::endl;
      }
      //assertTest( testcase );     -- Causes crash
   }
   bool mThreadAssertTest; // true for no error

   virtual ~SocketTest()
   {
   }

   
         
   // =========================================================================
   // open-close-open test
   // =========================================================================
   void testOpenCloseOpen_connector( void* data )
   {
      int num_of_times_to_test = 9;
      vpr::Uint16 port = 6970;
      //const int backlog = 5;
      bool result = 0;
      
      // make a new socket that will connect to port "port"
      vpr::InetAddr remote_addr;
      remote_addr.setPort(port);
      vpr::SocketStream	connector_socket( vpr::InetAddr::AnyAddr, remote_addr );
      
      // run the test many times.
      for (int x = 0; x < num_of_times_to_test; ++x)
      {
         // open socket
         result = connector_socket.open().success();
         threadAssertTest( (result != false) && "Socket::open() failed" );

         // connect to the acceptor
         result = connector_socket.connect().success();
         threadAssertTest( result != false && "Socket::connect() failed" );

         // let acceptor accept it before closing
         vpr::System::msleep( 50 );
         
         // close the socket
         result = connector_socket.close().success();
         threadAssertTest( result != false && "Socket::close() failed" );

         // let the acceptor get a chance to start before connecting (sleep a while)
         vpr::System::usleep( 5000 );
      }
   }
   void testOpenCloseOpen_acceptor( void* data )
   {
      int num_of_times_to_test = 3;
      vpr::Uint16 port = 6970;
      bool result = 0;

      vpr::InetAddr local_addr;

      // make a new socket listening on port "port"
      local_addr.setPort(port);
      vpr::SocketStream	acceptor_socket( local_addr, vpr::InetAddr::AnyAddr );
      
      // start/stop the acceptor many times...
      for (int x = 0; x < num_of_times_to_test; ++x)
      {
         //std::cout << "[acceptor open]" << std::flush;

         // open socket
         result = acceptor_socket.open().success();
         threadAssertTest( result != false && "Socket::open() failed" );

         result = acceptor_socket.bind().success();
         threadAssertTest( result != false && "Socket::bind() failed" );

         // set the socket to listen
         result = acceptor_socket.listen().success();
         threadAssertTest( result != false && "Socket::listen() failed" );

         //std::cout << "[accepting "<<num_of_times_to_test<<" connections]" << std::flush;

         vpr::SocketStream child_socket;

         // start/stop the child socket many times...
         for (int xx = 0; xx < num_of_times_to_test; ++xx)
         {
            //std::cout << x* num_of_times_to_test + xx << "+\n" << std::flush;
            //std::cout << "+" << std::flush;

            // wait for a connect (blocking)
            // when someone connects to the server, and we accept the connection, 
            // spawn a child socket to deal with the connection
            result = acceptor_socket.accept(child_socket).success();
            threadAssertTest( result && "Socket::accept() failed" );

            // close the child socket
            result = child_socket.close().success();
            threadAssertTest( result != false && "Socket::close() failed" );
         }

         // close the socket
         result = acceptor_socket.close().success();
         threadAssertTest( result != false && "Socket::close() failed" );

         //std::cout << "[acceptor close]\n" << std::flush;
      }
   }
   void testOpenCloseOpen()
   {
      //std::cout<<"]==================================================\n"<<std::flush; 
      //std::cout<<" OpenCloseOpen Test: \n"<<std::flush; 
      
      // spawn an acceptor thread
      vpr::ThreadMemberFunctor<SocketTest> acceptor_functor( this, &SocketTest::testOpenCloseOpen_acceptor );
      vpr::Thread acceptor_thread( &acceptor_functor );
      
      // let the acceptor get a chance to start before connecting (sleep a while)
      vpr::System::msleep( 500 );
      
      // spawn a connector thread
      vpr::ThreadMemberFunctor<SocketTest> connector_functor( this, &SocketTest::testOpenCloseOpen_connector );
      vpr::Thread connector_thread( &connector_functor );
      
      // wait for both threads to terminate, then continue
      //vpr::System::sleep( 7 );
      acceptor_thread.join();
      connector_thread.join();
      
      assertTest( mThreadAssertTest && "one of the threads asserted" );
      mThreadAssertTest = true; // reset it
      
      std::cout << " done\n" << std::flush;

      //connector_thread.join();
      //acceptor_thread.join();
      
   }
   // =========================================================================
   
   
   // =========================================================================
   // send-recv test
   // =========================================================================
   const std::string testSendRecv_buffer;
   void testSendRecv_connector( void* data )
   {
      int num_of_times_to_test = 10;
      vpr::Uint16 port = 6940;
      //const int backlog = 5;
      bool result = 0;
      vpr::InetAddr remote_addr;

      // make a new socket that will connect to port "port"
      remote_addr.setPort(port);
      vpr::SocketStream	connector_socket( vpr::InetAddr::AnyAddr, remote_addr );
      
      // run the test many times.
      for (int x = 0; x < num_of_times_to_test; ++x)
      {
         // open socket
         result = connector_socket.open().success();
         threadAssertTest( result != false && "Socket::open() failed" );

         // connect to the acceptor
         result = connector_socket.connect().success();
         threadAssertTest( result != false && "Socket::connect() failed" );

            // find out how much data is coming...
            int size_of_data;
            ssize_t amount_read;
            result = connector_socket.readn(&size_of_data, sizeof(int),
                                            amount_read).success();
            threadAssertTest( result && "readn didn't read" );
            threadAssertTest( amount_read == sizeof( int ) && "readn didn't read all" );
            
            // get that amount of data...
            std::string buffer;
            buffer.resize( size_of_data );
            result = connector_socket.readn(&buffer[0], size_of_data,
                                            amount_read).success();
            threadAssertTest( result && "readn didn't read" );
            threadAssertTest( amount_read == size_of_data && "readn didn't read all" );
            
            //std::cout<<"Recieved buffer: "<<buffer<<"\n"<<std::flush;
            threadAssertTest( buffer == testSendRecv_buffer && "didn't recieve the right data" );
            
         // close the socket
         result = connector_socket.close().success();
         threadAssertTest( result != false && "Socket::close() failed" );

         // let the acceptor get a chance to start before connecting (sleep a while)
         vpr::System::usleep( 50000 );
      }
   }
   void testSendRecv_acceptor( void* data )
   {
      int num_of_times_to_test = 10;
      vpr::Uint16 port = 6940;
      bool result = 0;
      vpr::InetAddr local_addr;

      // make a new socket listening on port "port"
      local_addr.setPort(port);
      vpr::SocketStream	acceptor_socket( local_addr, vpr::InetAddr::AnyAddr );
      
      // open socket
      result = acceptor_socket.open().success();
      threadAssertTest( result != false && "Socket::open() failed" );

      result = acceptor_socket.bind().success();
      threadAssertTest( result != false && "Socket::bind() failed" );

      // set the socket to listen
      result = acceptor_socket.listen().success();
      threadAssertTest( result != false && "Socket::listen() failed" );

      vpr::SocketStream child_socket;

      // start/stop the child socket many times...
      for (int xx = 0; xx < num_of_times_to_test; ++xx)
      {
         // wait for a connect (blocking)
         // when someone connects to the server, and we accept the connection, 
         // spawn a child socket to deal with the connection
         result = acceptor_socket.accept(child_socket).success();
         threadAssertTest( result && "Socket::accept() failed" );

            // send the size of the data to be sent
            ssize_t amount_sent;
            std::string buffer = testSendRecv_buffer;
            int size = buffer.size();
            result = child_socket.write( &size, sizeof( int ), amount_sent ).success();
            threadAssertTest( result && "write didn't send" );
            threadAssertTest( amount_sent == sizeof( int ) && "write didn't send all" );
            
            // send the data...
            result = child_socket.write( &buffer[0], buffer.size(), amount_sent).success();
            threadAssertTest( result && "write didn't send" );
            threadAssertTest( amount_sent == (int)buffer.size() && "write didn't send all" );
            
            //std::cout<<"Sent buffer: "<<buffer<<"\n"<<std::flush;
            //std::cout<<"+"<<std::flush;
         
         // close the child socket
         result = child_socket.close().success();
         threadAssertTest( result != false && "Socket::close() failed" );
      }

      // close the socket
      result = acceptor_socket.close().success();
      threadAssertTest( result != false && "Socket::close() failed" );
   }
   void testSendRecv()
   {
      //std::cout<<"]==================================================\n"<<std::flush; 
      //std::cout<<" SendRecv Test: \n"<<std::flush; 

      // spawn an acceptor thread
      vpr::ThreadMemberFunctor<SocketTest> acceptor_functor( this, &SocketTest::testSendRecv_acceptor );
      vpr::Thread acceptor_thread( &acceptor_functor );
      
      // let the acceptor get a chance to start before connecting (sleep a while)
      vpr::System::msleep( 500 );
      
      // spawn a connector thread
      vpr::ThreadMemberFunctor<SocketTest> connector_functor( this, &SocketTest::testSendRecv_connector );
      vpr::Thread connector_thread( &connector_functor );
      
      // wait for both threads to terminate, then continue
      //vpr::System::sleep( 7 );
      connector_thread.join(); // join is broken.
      acceptor_thread.join();
      
      assertTest( mThreadAssertTest && "one of the threads asserted" );
      mThreadAssertTest = true; // reset it
            
   }
   // =========================================================================
   
   
   // =========================================================================
   // open-close test
   // =========================================================================
   void testOpenClose()
   { 
      //std::cout<<"]==================================================\n"<<std::flush; 
      //std::cout<<" OpenClose Test: \n"<<std::flush; 

      bool openSuccess( false );
      bool closeSuccess( false );
      bool bindSuccess( false );
      
      vpr::Uint16 port=15432;
      vpr::InetAddr local_addr;
      vpr::SocketStream*	sock;

      local_addr.setAddress("localhost", port);
      sock = new vpr::SocketStream(local_addr, vpr::InetAddr::AnyAddr);	
      openSuccess=sock->open().success();
      if (openSuccess){
         //std::cout<< " Open...";
         bindSuccess=(sock->bind().success());
      }
      closeSuccess=sock->close().success();
      
      assertTest( openSuccess && "Socket can not be opened!");
      assertTest( bindSuccess && "Socket can not be bound!");
      assertTest( closeSuccess && "Socket can not be closed!");
      delete sock;
   }
   
   // =========================================================================
   // bind again should fail test
   // =========================================================================
   void bindAgainFailTest()
   { 
      //std::cout<<"]==================================================\n"<<std::flush; 
      //std::cout<<" multiple bind failure Test: \n"<<std::flush; 
      bool openSuccess( false );
      bool closeSuccess( false );
      bool bindSuccess( false );
      vpr::InetAddr local_addr;
      vpr::Uint16 port = 6976;

      local_addr.setAddress("localhost", port);
      vpr::SocketStream sock( local_addr, vpr::InetAddr::AnyAddr );	
      
      // make sure aditional calls to bind() fails...
      openSuccess = sock.open().success();
      assertTest( openSuccess == true && "open() failed");
      
      bindSuccess = sock.bind().success();
      assertTest( bindSuccess == true && "bind() failed");
      
      for (int x = 0; x < 2; ++x)
      {
         bindSuccess = sock.bind().success();
         assertTest( bindSuccess == false && "Socket was able to bind again, this is bad.");
      }
      
      closeSuccess = sock.close().success();
      assertTest( closeSuccess == true && "close() failed");
   }
   // =========================================================================
   // same-address-open-bind-close test
   // =========================================================================
   void sameAddressOpenBindCloseTest()
   { 
      //std::cout<<"]==================================================\n"<<std::flush; 
      //std::cout<<" same-address-open-bind-close Test: \n"<<std::flush; 
      int openSuccess( 0 );
      int closeSuccess( 0 );
      int bindSuccess( 0 );
      vpr::InetAddr local_addr;
      vpr::Uint16 port = 6977;

      local_addr.setAddress("localhost", port);
      vpr::SocketStream sock( local_addr, vpr::InetAddr::AnyAddr );	
      
      // same address, open-bind-close
      const int runs = 10;
      for (int xx = 0; xx < runs; ++xx)
      {
         openSuccess += sock.open().success() ? 1 : 0;
         bindSuccess += sock.bind().success() ? 1 : 0;
         closeSuccess += sock.close().success() ? 1 : 0;
      }
      const int success_percent = 80;
      int minimum_for_success = runs * success_percent / 100;
      //std::cout<<"out of ["<<runs<<"] runs: open="<<openSuccess<<" bind="<<bindSuccess<<" close="<<closeSuccess<<"\n"<<std::flush;
      assertTest( openSuccess >= minimum_for_success && "open() failed");
      assertTest( bindSuccess >= minimum_for_success && "bind() failed");
      assertTest( closeSuccess >= minimum_for_success && "close() failed");
   }
   // =========================================================================
   // different-address-open-bind-close test
   // - Try to open, close, and bind on a large range of ports
   // =========================================================================
   void differentAddressOpenBindCloseTest()
   { 
      //std::cout<<"]==================================================\n"<<std::flush; 
      //std::cout<<" different-address-open-bind-close Test: \n"<<std::flush; 
      int openSuccess( 0 );
      int closeSuccess( 0 );
      int bindSuccess( 0 );
      vpr::InetAddr local_addr;
      vpr::Uint16 port;

      // same address, open-bind-close
      const int runs = 10;
      for (int xx = 0; xx < runs; ++xx)
      {
         port = 5977 + xx;
	 assertTest(local_addr.setAddress("localhost", port));

         vpr::SocketStream sock( local_addr, vpr::InetAddr::AnyAddr );	
      
         openSuccess += sock.open().success() ? 1 : 0;
         bindSuccess += sock.bind().success() ? 1 : 0;
         closeSuccess += sock.close().success() ? 1 : 0;
      }
      const int success_percent = 80;
      int minimum_for_success = runs * success_percent / 100;
      //std::cout<<"out of ["<<runs<<"] runs: open="<<openSuccess<<" bind="<<bindSuccess<<" close="<<closeSuccess<<"\n"<<std::flush;
      assertTest( openSuccess >= minimum_for_success && "open() failed");
      assertTest( bindSuccess >= minimum_for_success && "bind() failed");
      assertTest( closeSuccess >= minimum_for_success && "close() failed");
   }
   // =========================================================================
   // reuse address simple test
   // =========================================================================
   
   void reuseAddrSimpleTest()
   {
      //std::cout<<"]==================================================\n"<<std::flush; 
      //std::cout<<" Reuse Address Test (simple version)"<<endl;
      vpr::InetAddr addr1;
      //vpr::InetAddr addr2;
      vpr::SocketStream*	sock1;
      vpr::SocketStream*	sock2;
      vpr::SocketStream*	sock3;

      addr1.setPort(13768);
      //assertTest(addr2("129.186.232.58", 5438));

      sock1 = new vpr::SocketStream(addr1, vpr::InetAddr::AnyAddr);
      sock2 = new vpr::SocketStream(addr1, vpr::InetAddr::AnyAddr);
      sock3 = new vpr::SocketStream(addr1, vpr::InetAddr::AnyAddr);
      if (sock1->open().success()){
         sock1->setReuseAddr(true);
         assertTest(sock1->bind().success());
      }
      else assert(false && "Cannot open sock1");
      if (sock2->open().success()){
         assertTest(sock2->bind().success() && "Cannot bind second socket re-using addr");
      }
      else assert(false && "Cannot open sock2");
      if (sock3->open().success())
         assertTest(sock3->bind().success() && "Cannot bind third socket re-using addr");
      sock1->close();
      sock2->close();
      sock3->close();
      delete sock1;
      delete sock2;
      delete sock3;
   }


   // =========================================================================
   // reuse address client/server test
   // =========================================================================
   void reuseAddrTest_connector( void* data )
   {
      vpr::Uint16 port = 6667;
      bool result = 0;
      vpr::InetAddr remote_addr;

      // make a new socket that will connect to port "port"
      remote_addr.setPort(port);
      vpr::SocketStream	connector_socket( vpr::InetAddr::AnyAddr, remote_addr );

      // open socket
      result = connector_socket.open().success();
      threadAssertTest( result != false && "Socket::open() failed" );

      // connect to the acceptor
      result = connector_socket.connect().success();
      threadAssertTest( result != false && "Socket::connect() failed" );

      // close the socket
      result = connector_socket.close().success();
      threadAssertTest( result != false && "Socket::close() failed" );
   }   
   void reuseAddrTest_acceptor( void* data )
   {
      vpr::InetAddr addr1;
      
      threadAssertTest(addr1.setAddress( "localhost", 6667 ));

      vpr::SocketStream sock1( addr1, vpr::InetAddr::AnyAddr );
      vpr::SocketStream sock2( addr1, vpr::InetAddr::AnyAddr );

      assertTest( sock1.open().success() && "server Socket::open() failed" );
      sock1.setReuseAddr( true );
      threadAssertTest( sock1.bind().success() && "server Socket::bind() failed" );
      threadAssertTest( sock1.listen().success() && "server Socket::listen() failed" );
      
      vpr::SocketStream child_socket;
      vpr::Status status = sock1.accept(child_socket);
      threadAssertTest( status.success() && "child didn't spawn" );

         // assume server crashes, so lets restart it.
         threadAssertTest( sock2.open().success() && "open(): server restart" );
         sock1.setReuseAddr( true ); // set the opt in-between for bind() to succeed
         bool result = sock2.bind().success();
         //std::cout<<"::::::::: " << result<<"\n"<<std::flush;
         threadAssertTest( result == true && "bind(): server restart" );
         //std::cout<<"::::::::: " << result<<"\n"<<std::flush;
         
      // close the child socket
      threadAssertTest( child_socket.close().success() && "child Socket::close() failed" );

      threadAssertTest( sock2.close().success() && "restarted server Socket::close() failed" );
      threadAssertTest( sock1.close().success() && "server Socket::close failed");
   }
   
   // XXX: Fails due to crashing thread (I think it is the connector thread)
   void reuseAddrTest()
   {
      //std::cout<<"]==================================================\n"<<std::flush; 
      //std::cout<<" reuseAddr Test (cli/serv version): \n"<<std::flush; 
      
      // spawn an acceptor thread
      vpr::ThreadMemberFunctor<SocketTest> acceptor_functor( this, &SocketTest::reuseAddrTest_acceptor );
      vpr::Thread acceptor_thread( &acceptor_functor );
      
      // let the acceptor get a chance to start before connecting (sleep a while)
      vpr::System::msleep( 500 );
      
      // spawn a connector thread
      vpr::ThreadMemberFunctor<SocketTest> connector_functor( this, &SocketTest::reuseAddrTest_connector );
      vpr::Thread connector_thread( &connector_functor );
      
      // wait for both threads to terminate, then continue
      //vpr::System::sleep( 1 );
      
      acceptor_thread.join();
      connector_thread.join(); // join is broken.

      assertTest( mThreadAssertTest && "one of the threads asserted" );
      mThreadAssertTest = true; // reset it
      
      //std::cout << " done\n" << std::flush;
   }
   
   // =========================================================================
   // Blocking/Nonblocking test
   // =========================================================================
   void testBlocking_connector(void* arg)
   {
      vpr::Uint16 port = 7001;
      vpr::InetAddr remote_addr;

      remote_addr.setPort(port);

      //keep testing until acceptor says finish
      while (mFinishFlag == false) {
         char    buffer[40];
         bool    result = 0;
         char    buffer2[]="Oops!";
         ssize_t amount_read, amount_written;
         
         // make a new socket that will connect to port "port"
         vpr::SocketStream connector_socket(vpr::InetAddr::AnyAddr,
                                            remote_addr);
      
         // open socket
         connector_socket.setOpenBlocking();
         result = connector_socket.open().success();
         threadAssertTest( result != false && "Socket::open() failed when test blocking" );

         // connect to the acceptor
         result = connector_socket.connect().success();
         threadAssertTest( result != false && "Socket::connect() failed when test blocking" );
         
         connector_socket.setNoDelay(true);
         
         //sleep until acceptor says begin
         while (mStartFlag == false){
            vpr::System::usleep(50);
         }
         
         //set socket to blocking/nonblocking mode as required          
         if (mBlockingFlag == true)
            connector_socket.enableBlocking();
         else
            connector_socket.enableNonBlocking();
         
         //if the the flag of readn is true, use readn(), otherwise use read().
         if (mReadnFlag == true)
            connector_socket.readn(buffer, 20, amount_read);
         else
            connector_socket.read(buffer, 20, amount_read);
            
         if (amount_read>=0) {
            connector_socket.write(buffer, amount_read, amount_written);
         }
         else {
            connector_socket.write(buffer2, sizeof(buffer2), amount_written);
         }
         
         while (mStartFlag == true){
            vpr::System::usleep(50);
         }
         
         result = connector_socket.close().success();
         threadAssertTest( result != false && "Socket::close() failed when test blocking" );         
      }

      // let the acceptor get a chance to start before connecting (sleep a while)
      vpr::System::usleep( 50000 );
   }

   void testBlocking_acceptor(void* arg)
   {
      int num_of_times_to_test = 20;
      vpr::Uint16 port = 7001;
      bool  result = 0;
      char  buffer[40];
      char  buffer1[]="Hello, there!";
      char  buffer2[]="Hello";
      ssize_t amount_read (0), amount_written;
      vpr::InetAddr local_addr;

      local_addr.setPort(port);

      // make a new socket listening on port "port"
      vpr::SocketStream	acceptor_socket( local_addr, vpr::InetAddr::AnyAddr );
      acceptor_socket.setOpenBlocking();
      
      // open socket
      result = acceptor_socket.open().success();

      threadAssertTest( result != false && "Socket::open() failed in blocking test" );

      result = acceptor_socket.bind().success();
      threadAssertTest( result != false && "Socket::bind() failed in blocking test" );
      
      mFinishFlag = false;
      
      // set the socket to listen
      result = acceptor_socket.listen().success();
      threadAssertTest( result != false && "Socket::listen() failed in blocking test" );
      
      
      vpr::SocketStream child_socket;

      // switch between blocking/nonblocking and read/readn many times...
      for (int xx = 0; xx < num_of_times_to_test; xx++)
      {
         // wait for a connect (blocking)
         // when someone connects to the server, and we accept the connection, 
         // spawn a child socket to deal with the connection
         
         int yy=xx%4;
         
         result = acceptor_socket.accept(child_socket).success();
         threadAssertTest( result && "Socket::accept() failed in blocking test" );
         
         mFlagProtectionMutex.acquire();
            if(yy==0 || yy==2)
               mBlockingFlag=true;
            else mBlockingFlag=false;
         
            if(yy==0 || yy==1)
               mReadnFlag=false;
            else mReadnFlag=true;
            mStartFlag=true;
         mFlagProtectionMutex.release();
                 
         vpr::System::msleep(50);
         
         child_socket.write(buffer1, sizeof(buffer1), amount_written);
         
         vpr::System::msleep(5);
         
         child_socket.write(buffer2, sizeof(buffer2), amount_written);
         
         child_socket.enableNonBlocking();
         child_socket.read(buffer, 40, amount_read);
         child_socket.enableBlocking();

         switch (yy) {
         case 0:
            threadAssertTest(amount_read == 14 && "Should return 14");
            break;         
         case 1:
            threadAssertTest(amount_read == 6 && "Should return 6");
            break;
         case 2:
            threadAssertTest(amount_read == -1 && "Should return 20");
            break;
         case 3:
            threadAssertTest(amount_read==6 && "Should return 6");
         }
         
         mFlagProtectionMutex.acquire();         
         mStartFlag=false;
         mFlagProtectionMutex.release();
         
         result = child_socket.close().success();
         threadAssertTest( result != false && "Socket::close() failed in blocking test" );
      }
      
      // close the socket
      mFlagProtectionMutex.acquire();
         mFinishFlag = true;
      mFlagProtectionMutex.release();
      result = acceptor_socket.close().success();
      threadAssertTest( result != false && "Socket::close() failed" );
   }  
   
   void testBlocking()
   {
      //std::cout<<"]==================================================\n"<<std::flush; 
      //std::cout<<" Blocking/Nonblocking Test: \n"<<std::flush; 
      
      // spawn an acceptor thread
      vpr::ThreadMemberFunctor<SocketTest> acceptor_functor( this, &SocketTest::testBlocking_acceptor );
      vpr::Thread acceptor_thread( &acceptor_functor );
      
      // let the acceptor get a chance to start before connecting (sleep a while)
      vpr::System::msleep( 500 );
      
      // spawn a connector thread
      vpr::ThreadMemberFunctor<SocketTest> connector_functor( this, &SocketTest::testBlocking_connector );
      vpr::Thread connector_thread( &connector_functor );
      
      // wait for both threads to terminate, then continue

      //vpr::System::sleep( 10 );
      connector_thread.join(); // join is broken.
      acceptor_thread.join();
      
      assertTest( mThreadAssertTest && "blocking test: one of the threads asserted" );
      mThreadAssertTest = true; // reset it
      
      //std::cout << " done\n" << std::flush;
   }
   
            
   void testTcpConnection()
   {
      mServerCheck=0;
      //Creat a thread to do server listen:
      vpr::ThreadMemberFunctor<SocketTest>* serverFunctor;
      serverFunctor=new vpr::ThreadMemberFunctor<SocketTest>(this, &SocketTest::serverFunc);
      vpr::Thread* serverThread;
      serverThread=new vpr::Thread(serverFunctor);

      vpr::System::sleep(1);

      //Creat a bunch of client thread.
      std::vector<vpr::ThreadMemberFunctor<SocketTest>*> clientFunctors(mNumClient);
      std::vector<vpr::Thread*> clientThreads(mNumClient);
      for (int t=0; t<mNumClient; t++){
         clientFunctors[t] = new vpr::ThreadMemberFunctor<SocketTest>(this, &SocketTest::clientFunc);
         clientThreads[t] = new vpr::Thread(clientFunctors[t]);
      }

      vpr::System::sleep(1);
      //serverThread->join();

	   assertTest(mOpenServerSuccess ==-1 && "Open server failed");
      assertTest(mServerCheck==0 && "Not all connections are correct.");
      delete serverFunctor;
      delete serverThread;

      for (int t=0; t<mNumClient; t++){
         delete clientFunctors[t];
         delete clientThreads[t];
      }
   }

   //Function for master server thread
   void serverFunc(void* arg)
   {
      vpr::Uint16 port=15432;
      vpr::Uint16 num=0;

      std::vector<vpr::ThreadMemberFunctor<SocketTest>*> sServerFunctors(mNumSServer);
      std::vector<vpr::Thread*> sServerThreads(mNumSServer);
      vpr::InetAddr local_addr;

      local_addr.setPort(port);
      vpr::SocketStream*	sock;
      sock = new vpr::SocketStream(local_addr, vpr::InetAddr::AnyAddr);	
      if ( sock->openServer().success() ) {
         vpr::SocketStream client_sock;
         thread_args_t* tArgs;

         while ( num<mNumSServer) {
            sock->accept(client_sock);
            tArgs = new thread_args_t;
            tArgs->mSock= new vpr::SocketStream(client_sock);

            sServerFunctors[num]=new vpr::ThreadMemberFunctor<SocketTest>(this, &SocketTest::sServerFunc, tArgs);
            sServerFunctors[num]->setArg(tArgs);
            sServerThreads[num]= new vpr::Thread(sServerFunctors[num]);

            num++;
         }
         mOpenServerSuccess =-1;
      }
      else {
         mOpenServerSuccess = 1;
      }
      sock->close();
      delete sock;
   }
   
   //function for creating client threads
   void clientFunc(void* arg)
   {
      vpr::SocketStream*	sock;
      vpr::InetAddr remote_addr;

      remote_addr.setAddress("localhost", 15432);
      sock = new vpr::SocketStream(vpr::InetAddr::AnyAddr, remote_addr);
      if ( sock->open().success() ) {
         char buffer1[40];
         //char buffer2[] = "What's up?";
         if ( sock->connect().success() ) {
            ssize_t bytes, bytes_written;
            sock->read(buffer1, 40, bytes);
         //sock->write(buffer2, sizeof(buffer2), bytes_written);
            sock->write(buffer1, bytes, bytes_written);
            mItemProtectionMutex.acquire();
            mClientCounter++;
            mItemProtectionMutex.release();
         }
      }
      sock->close();
      delete sock;
   }
   
   //function for creating slave server thread, the server sends message to client, gets the client's 
   //echo, and then compares the two strings.
   void sServerFunc(void* arg)
   {
      _thread_args* tArg=(_thread_args *) arg;
      char buffer1[] = "Hello there!";
      char buffer2[40];
      ssize_t bytes, bytes_written;

      vpr::SocketStream* ss_sock;
      ss_sock=tArg->mSock;
      //send a string to client
      ss_sock->write(buffer1, sizeof(buffer1), bytes_written);
      //receive a string from client
      ss_sock->read(buffer2,40, bytes);
      long compareString=strcmp(buffer1,buffer2);
      if (compareString!=0) {
         mServerCheck=1;
      }
      ss_sock->close();
      delete ss_sock;
      
      // Deleting void* is not "good"
      //delete arg;
   }

   // =========================================================================
   // readn() test
   // =========================================================================
   void testReadnClient (void* arg) {
      vpr::Uint16 port = *((vpr::Uint16*) arg);
      vpr::InetAddr remote_addr;

      threadAssertTest(remote_addr.setAddress("localhost", port) && "Could not assign address");
      vpr::SocketStream client_sock(vpr::InetAddr::AnyAddr, remote_addr);
      char buffer[20];

      threadAssertTest(client_sock.open().success() && "Client socket open failed");
      threadAssertTest(client_sock.connect().success() && "Client could not connect");
      ssize_t bytes;
      client_sock.readn(buffer, sizeof(buffer), bytes);
      threadAssertTest((bytes == sizeof(buffer)) && "readn didn't read enough!");
      client_sock.close();
   }

   void testReadn () {
      vpr::Uint16 server_port = 55446;
      vpr::InetAddr local_addr;
      local_addr.setPort(server_port);
      vpr::SocketStream server_sock(local_addr, vpr::InetAddr::AnyAddr);
      const unsigned int pkt_size = 5;
      ssize_t bytes;
      char buffer[pkt_size];

      //std::cout << "]==================================================\n"
      //          << std::flush; 
      //std::cout <<" Readn Test:\n" << std::flush; 

      assertTest(server_sock.open().success() && "Server socket open failed");
      assertTest(server_sock.bind().success() && "Server socket bind failed");
      assertTest(server_sock.listen().success() && "Server socket listen failed");

      // Start the client thread.
      vpr::ThreadMemberFunctor<SocketTest> func =
          vpr::ThreadMemberFunctor<SocketTest>(this, &SocketTest::testReadnClient,
                                               (void*) &server_port);
      vpr::Thread* client_thread = new vpr::Thread(&func);
      assertTest(client_thread->valid() && "Server could not create client thread");

      vpr::SocketStream client_sock;
      vpr::Status status = server_sock.accept(client_sock);
      assertTest(status.success() && "Server could not accept connection");

      for ( unsigned int i = 0; i < 20; i += pkt_size ) {
          memset((void*) buffer, 0, sizeof(buffer));
          snprintf(buffer, sizeof(buffer), "%04d", i);

          client_sock.write(buffer, pkt_size, bytes);
          assertTest(bytes != -1 && "Server could not write to client");
      }

      client_sock.close();
      delete client_thread;
      server_sock.close();
   }
   // =========================================================================

   static Test* suite()
   {
      TestSuite *test_suite = new TestSuite ("SocketTest");

      test_suite->addTest( new TestCaller<SocketTest>("testOpenClose", &SocketTest::testOpenClose));      
      test_suite->addTest( new TestCaller<SocketTest>("bind-Again Failure Test", &SocketTest::bindAgainFailTest));
      test_suite->addTest( new TestCaller<SocketTest>("same-Address-Open-Bind-Close Test", &SocketTest::sameAddressOpenBindCloseTest));
      test_suite->addTest( new TestCaller<SocketTest>("different-Address-Open-Bind-Close Test", &SocketTest::differentAddressOpenBindCloseTest));
      
      test_suite->addTest( new TestCaller<SocketTest>("ReuseAddr (simple) Test", &SocketTest::reuseAddrSimpleTest));
      test_suite->addTest( new TestCaller<SocketTest>("ReuseAddr (client/server) Test", &SocketTest::reuseAddrTest));

      test_suite->addTest( new TestCaller<SocketTest>("testOpenCloseOpen", &SocketTest::testOpenCloseOpen));
      test_suite->addTest( new TestCaller<SocketTest>("testSendRecv", &SocketTest::testSendRecv));

      test_suite->addTest( new TestCaller<SocketTest>("testBlocking", &SocketTest::testBlocking));
      test_suite->addTest( new TestCaller<SocketTest>("testTcpConnection", &SocketTest::testTcpConnection));
      test_suite->addTest( new TestCaller<SocketTest>("testReadn", &SocketTest::testReadn));
      return test_suite;
   }

protected:
   vpr::Mutex     mItemProtectionMutex;         // Protect an exclusive item
   vpr::Mutex     mFlagProtectionMutex;         // Protect an flags using in blocking test
   bool           mFinishFlag;
   bool           mBlockingFlag;
   bool           mStartFlag;
   bool           mReadnFlag;
   long           mOpenServerSuccess;
   long           mNumSServer;
   long           mNumClient;
   long           mClientCounter;
   long           mServerCheck;
};

}

#endif          

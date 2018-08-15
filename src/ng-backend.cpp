#include <iostream>

extern "C"
{
  #include <unistd.h> //daemon()
  #include <syslog.h> //Will be use... Probably in his own object.
}

#include <csignal> // signal() : To properly free all the ressources.

#include "args.hpp"
#include "Audio-Bridge-Server.hpp"
#include "AudioDatabase.hpp"

AudioServer   *Server = nullptr;
AudioDatabase *db     = nullptr;

//You really have to check your privilege !
static void drop_privilege(const char* user, const char* group)
{
  //Write the chg user stuff here (IF Root)
}

static void signal_handler( int sig )
{
  switch( sig )
  {
    case SIGINT:  
      if( Server != nullptr )
        Server->stop();
      
      std::cout << "Server stopped..." << std::endl;
      break;
    
    case SIGUSR1:
      if(db)
        db->reload();
      std::cout << "[+] Database reloaded" << std::endl;
      break;
    
    default:;
  }
}

static void create_objects_via_args(int argc, char **argv)
{
  cArg argument(argc, argv);
  
  db = new AudioDatabase();
  if( !db->openDBFile(argument.getDBPath()) )
  {
    std::cerr << "Can't open DB File. Exiting..." << std::endl;
    std::exit(-1);
  }
  
  Server = new AudioServer( argument.getListenIP(),
                            argument.getListeningPort());
  Server->setDBController(db);
  
  std::cout << "Server listenning on " << argument.getListenIP() << ":" << argument.getListeningPort() << std::endl;
  
  if( argument.daemonize() )
  {
    daemon(0, 0);
    drop_privilege( argument.getUserName().c_str(),
                    argument.getGroupName().c_str() );
  }
}

int main( int argc, char *argv[] )
{
  signal(SIGINT, signal_handler);
  
  create_objects_via_args(argc, argv);
  
  std::cout << "Entering server loop..." << std::endl;
  Server->serve();
  
  delete Server;
  delete db;
  
  return 0;
}

#include <iostream>

extern "C"
{
  #include <unistd.h> //daemon()
  #include <syslog.h> //Will be use... Probably in his own object.
  
  #include <sys/types.h>
  #include <pwd.h>
  #include <grp.h>
}

#include <csignal> // signal() : To properly free all the ressources.

#include "args.hpp"
#include "Audio-Bridge-Server.hpp"
#include "AudioDatabase.hpp"

AudioServer   *Server = nullptr;
AudioDatabase *db     = nullptr;

//You really have to check your privilege !
static void drop_privilege(const char* user)
{
  if( user && getuid() == 0 )
  {
    struct passwd *pw = getpwnam(user);
    if(pw)
    {
      if( initgroups( pw->pw_name, pw->pw_gid ) != 0 || setgid( pw->pw_gid ) !=0 || setuid( pw->pw_uid ) != 0 )
      {
        std::cerr << "[-] Couldn't change to " << user << " uid=" << pw->pw_uid << " gid=" << pw->pw_gid <<std::endl;
        std::exit(-1);
      }
      
      if( setuid(0) != -1 || setgid(0) != -1 )
      {
        std::cout << "[-] CRITICAL: Can become root again after dropping privilege..." << std::endl;
        std::exit(-1);
      }
    }
    else
    {
      std::cerr << "[-] Can't drop root privilege..." << std::endl;
      std::exit(-1);
    }
  }
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
  if( !db->openDBFile(argument.getDBPath(), argument.isLive()) )
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
    drop_privilege( argument.getUserName().c_str() );
    daemon(0, 0);
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

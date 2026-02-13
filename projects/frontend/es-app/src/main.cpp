//EmulationStation, a graphical front-end for ROM browsing. Created by Alec "Aloshi" Lofquist.
//http://www.aloshi.com

#include <EmulationStation.h>
#include <utils/Log.h>
#include "MainRunner.h"
#include "Options.h"

#include <bluetooth/DeviceStatus.h>
#include <sys/resource.h>

int main(int argc, char* argv[], char** env)
{
  // Rename main thread
  pthread_setname_np(pthread_self(), "Main");
  // file limit up !
  struct rlimit limit { 4096, 4096 };
  if (setrlimit(RLIMIT_NOFILE, &limit) != 0)
    { LOG(LogError) << "[Main] Error setting file limit."; }
  // Get arguments
  Options options(argc, argv);

  for(int loopCount = 0;; ++loopCount)
  {
    // Start the runner
    MainRunner runner(argv[0], loopCount, env, options);
    MainRunner::ExitState exitState = runner.Run();

    { LOG(LogInfo) << "[Main] EmulationStation cleanly shutting down."; }
    switch(exitState)
    {
      case MainRunner::ExitState::Quit:
      case MainRunner::ExitState::FatalError:
      {
        { LOG(LogInfo) << "[Main] Regular Quit (or Fatal Error)"; }
        return 0;
      }
      case MainRunner::ExitState::Relaunch:
      case MainRunner::ExitState::RelaunchNoUpdate: continue;
      case MainRunner::ExitState::NormalReboot:
      case MainRunner::ExitState::FastReboot:
      {
        { LOG(LogInfo) << "[Main] Rebooting system"; }
        #ifndef DEBUG
        if (system("reboot") != 0)
          { LOG(LogError) << "[Main] Error rebooting system"; }
        #else
        { LOG(LogError) << "[Main] Fake reboot performed!"; }
        #endif
        return 0;
      }
      case MainRunner::ExitState::Shutdown:
      case MainRunner::ExitState::FastShutdown:
      {
        { LOG(LogInfo) << "[Main] Shutting down system"; }
        #ifndef DEBUG
        if (system("shutdown -h now") != 0)
          { LOG(LogError) << "[Main] Error shutting system down"; }
        #else
        { LOG(LogError) << "[Main] Fake shutdown performed!"; }
        #endif
        return 0;
      }
    }
  }
}


#include <string.h>
#include <stdio.h>
#include <chrono>

#include <switch.h>

#include "gui.hpp"
#include "gui_main.hpp"

#include "threads.hpp"

static Gui *currGui = nullptr;
static bool updateThreadRunning = false;
static Mutex mutexCurrGui;


void update(void *args) {
  while (updateThreadRunning) {
    auto begin = std::chrono::steady_clock::now();

    mutexLock(&mutexCurrGui);
    if (currGui != nullptr)
      currGui->update();
    mutexUnlock(&mutexCurrGui);

    svcSleepThread(1.0E6 - std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - begin).count());
  }
}

int main(int argc, char **argv){
    u64 kdown = 0;
    gfxInitDefault();

    socketInitializeDefault();
    nxlinkStdio();

    setsysInitialize();
    ColorSetId colorSetId;
    setsysGetColorSetId(&colorSetId);
    setTheme(colorSetId);
    setsysExit();

    Gui::g_nextGui = GUI_MAIN;

    mutexInit(&mutexCurrGui);

    updateThreadRunning = true;
    Threads::create(&update);

    while(appletMainLoop()) {
      hidScanInput();
      kdown = hidKeysDown(CONTROLLER_P1_AUTO);

      if (Gui::g_nextGui != GUI_INVALID) {
        mutexLock(&mutexCurrGui);
        switch(Gui::g_nextGui) {
          case GUI_MAIN:
            currGui = new GuiMain();
            break;
        }
        mutexUnlock(&mutexCurrGui);
        Gui::g_nextGui = GUI_INVALID;
      }

      if(currGui != nullptr) {
        currGui->draw();

        if (kdown != 0)
          currGui->onInput(kdown);
      }
    }

    updateThreadRunning = false;
    Threads::joinAll();

    socketExit();

    gfxExit();
    return 0;
}
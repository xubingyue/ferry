#include "AppDelegate.h"
#include "HelloWorldScene.h"

USING_NS_CC;

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
#include <winsock2.h>
#pragma comment(lib,"pthreadVSE2.lib")
#define MY_SLEEP(sec) Sleep((sec)*1000);

#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#define MY_SLEEP(sec) sleep(sec);
#endif


AppDelegate::AppDelegate() {
    ferry::Ferry::getInstance()->addEventCallback(
            std::bind(&AppDelegate::eventCallback, this, std::placeholders::_1),
            this, "event_callback"
    );
}

AppDelegate::~AppDelegate() 
{
    ferry::Ferry::getInstance()->delCallbacksForTarget(this);
}

bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
        glview = GLView::create("My Game");
        director->setOpenGLView(glview);
    }

    // turn on display FPS
    director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0 / 60);

    // create a scene. it's an autorelease object
    auto scene = HelloWorld::createScene();

    // run
    director->runWithScene(scene);

    /*
    G::getFerry()->init(new MyFerryDelegate(), "127.0.0.1", 7777);
    G::getFerry()->start();

    G:getEventBus()->start();

    */


    ferry::Ferry::getInstance()->init("127.0.0.1", 7777);
    // ferry::Ferry::getInstance()->init("10.55.33.22", 7777);
    ferry::Ferry::getInstance()->start();

    // 启动心跳
    std::thread* hbThread = new std::thread(&AppDelegate::heartbeat,this);
    // 脱离
    hbThread->detach();

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    // SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    // SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}

void AppDelegate::eventCallback(ferry::Event *event) {
    cocos2d::log("event->what: %d", event->what);

    switch (event->what) {
        case ferry::EVENT_ERROR: {
            // 连接失败
            break;
        };
        case ferry::EVENT_TIMEOUT: {
            // 连接超时
            break;
        };
        case ferry::EVENT_CLOSE: {
            ferry::Ferry::getInstance()->connect();
            break;
        };
        case ferry::EVENT_OPEN: {
            auto func = [&](ferry::Event* event) {
                cocos2d::log("rsp, event->what: %d", event->what);

                // ferry::Ferry::getInstance()->disconnect();
                // ferry::Ferry::getInstance()->stop();
            };

            netkit::Box *box = new netkit::Box();
            box->version = 1;
            box->flag = 2;
            box->cmd = 1;
            box->setBody("aini");

            ferry::Ferry::getInstance()->send(box, func, 0.5, this);
            break;
        };
    }
}

void AppDelegate::heartbeat() {
    while(1) {
        netkit::Box* box = new netkit::Box();
        box->cmd = 7;

        // 必须已经连接的状态，才发送心跳
        if (ferry::Ferry::getInstance()->isConnected()) {
            ferry::Ferry::getInstance()->send(box);
        }

        // 检查超时
        if (ferry::Ferry::getInstance()->getLastActiveTime() > 0 &&
                time(NULL) - ferry::Ferry::getInstance()->getLastActiveTime() > 30)
        {
            // 说明保持连接超时了
            cocos2d::log("conn alive timeout, disconnect");
            ferry::Ferry::getInstance()->disconnect();
        }

        // 10秒
        MY_SLEEP(10);
    }
}


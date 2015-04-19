#include "AppDelegate.h"
#include "HelloWorldScene.h"

USING_NS_CC;

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
    ferry::Ferry::getInstance()->start();


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
        case ferry::EVENT_RECV: {
            cocos2d::log("lastRecvTime: %d", ferry::Ferry::getInstance()->getLastRecvTime());
            break;
        };
    }
}

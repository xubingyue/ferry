#include "AppDelegate.h"
#include "HelloWorldScene.h"
#include "Ferry.h"

USING_NS_CC;

AppDelegate::AppDelegate() {
    ferry::Ferry::getInstance()->addEventCallback(std::bind(&AppDelegate::eventCallback, this, std::placeholders::_1), this, "event_callback");
}

AppDelegate::~AppDelegate() 
{
    ferry::Ferry::getInstance()->delAllEventCallbacksForTarget(this);
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

    auto func = [this](float dt){
        G::getEventBus()->loopEvents();
    };

    cocos2d::Director::getInstance()->getScheduler()->schedule(func, this, 0, false, "eventbus_loop");
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

void AppDelegate::eventCallback(eventbus::BaseEvent* e) {
    cocos2d::log("e.what: %d", e->what);

    switch (e->what) {
        case ferry::EVENT_ON_CLOSE:
            ferry::Ferry::getInstance()->connect();
            break;
        case ferry::EVENT_ON_OPEN:
            auto func = [&](int code, netkit::Box* box) {
                cocos2d::log("code: %d", code);
            };

            netkit::Box *box = new netkit::Box();
            box->cmd = 1;
            box->setBody("aini", 4);

            ferry::Ferry::getInstance()->send(box, func, 10);
            break;
    }
}

//
// Created by QITMAC000260 on 2019-06-18.
//

#include "MacNotification.h"
#import <NotificationCenter/NotificationCenter.h>
#import <Foundation/Foundation.h>
#include "ChatViewMainPanel.h"
#include <QTimer>

extern ChatViewMainPanel *g_pMainPanel;

void sendTextMessage(const std::string &text,
        int chatType,
        const std::string &userId,
        const std::string& realJid,
        const std::string& loginUser) {

    if (g_pMainPanel && loginUser == g_pMainPanel->getSelfUserId()) {
        // 发送消息
        long long sendtime;
        sendtime = QDateTime::currentDateTime().toMSecsSinceEpoch() - DC.getServerDiffTime() * 1000;
        std::string msgId = st::utils::uuid();

        st::entity::ImMessageInfo message;
        message.ChatType = chatType;
        message.MsgId = msgId;
        message.To = userId;
        message.RealJid = realJid;
        message.From = g_pMainPanel->getSelfUserId();
        message.LastUpdateTime = sendtime;
        message.Type = st::entity::MessageTypeText;
        message.Content = text;
        message.Direction = st::entity::MessageDirectionSent;
        message.UserName = DC.getSelfName();

        S_Message e;
        e.message = message;
        e.userId = userId;
        e.time = sendtime;
        e.chatType = chatType;

        emit g_pMainPanel->sgUserSendMessage();
        ChatMsgManager::sendMessage(e);
//        showMessageSlot(message, false);
    }
}


@interface MacNotification : NSObject

+ (instancetype)instance;

@end

@interface  MacNotification () <NSUserNotificationCenterDelegate>
@end

@implementation MacNotification

- (void)showNotification :(st::StNotificationParam*)param {

    NSString *title = [NSString stringWithUTF8String:param->title.data()];
//    NSString *subTitle = [NSString stringWithUTF8String:param->subTitle];
    NSString *message = [NSString stringWithUTF8String:param->message.data()];
    NSString *image = [NSString stringWithUTF8String:param->icon.data()];

    NSUserNotification *notification = [[NSUserNotification alloc] init];
    // title
    [notification setTitle:title];
//    [notification setIdentifier:[NSString stringWithUTF8String:param->loginUser.data()]];
    // subTitle
//    if(nil != subTitle)
//        [notification setSubtitle:message];
    // message
    [notification setInformativeText:message];
    // image
    NSImage *headerImage = [self getImage:image];
    [notification setContentImage:headerImage];   //用户头像
    [headerImage release];
    // sound
//    if(param->playSound)
//        [notification setSoundName:@"sound/msg.wav"];
    // quick reply
    if ([notification respondsToSelector:@selector(setHasReplyButton:)])
    {
        [notification setHasReplyButton:YES];
        [notification setResponsePlaceholder:@"请输入回复内容:"];
    }
    notification.hasActionButton = NO;
    // user info
    NSDictionary *dictionary = @{@"xmppId": [NSString stringWithUTF8String:param->xmppId.data()],
                                 @"from": [NSString stringWithUTF8String:param->from.data()],
                                 @"realJid": [NSString stringWithUTF8String:param->realJid.data()],
                                 @"chatType": [NSString stringWithFormat:@"%d", param->chatType],
                                 @"loginUser": [NSString stringWithUTF8String:param->loginUser.data()],
                                 @"name": title};
    notification.userInfo = dictionary;

    NSUserNotificationCenter *userNotificationCenter = [NSUserNotificationCenter defaultUserNotificationCenter];
    [userNotificationCenter scheduleNotification:notification];
    [userNotificationCenter setDelegate:self];
    [notification release];
}

- (NSImage *)getImage:(NSString *)path {
    NSArray *imageReps = [NSBitmapImageRep imageRepsWithContentsOfFile:path];
    NSInteger width = 0;
    NSInteger height = 0;
    for (NSImageRep * imageRep in imageReps) {
        if ([imageRep pixelsWide] > width) width = [imageRep pixelsWide];
        if ([imageRep pixelsHigh] > height) height = [imageRep pixelsHigh];
    }
    NSImage *imageNSImage = [[NSImage alloc] initWithSize:NSMakeSize((CGFloat)width, (CGFloat)height)];
    [imageNSImage addRepresentations:imageReps];
    return imageNSImage;
}

- (void)userNotificationCenter:(NSUserNotificationCenter *)center didActivateNotification:(NSUserNotification *)notification
{
//    if (notification.activationType == NSUserNotificationActivationTypeNone)
//    {
//        NSLog(@"NSUserNotificationActivationTypeNone");
//    }
//    else
    NSString *xmppId = [notification.userInfo objectForKey:@"xmppId"];
    NSString *from = [notification.userInfo objectForKey:@"from"];
    NSString *realJid = [notification.userInfo objectForKey:@"realJid"];
    NSString *loginUser = [notification.userInfo objectForKey:@"loginUser"];
    int chatType = [[notification.userInfo objectForKey:@"chatType"] intValue];

    if (notification.activationType == NSUserNotificationActivationTypeContentsClicked)
    {
        NSString *name = [notification.userInfo objectForKey:@"name"];

        if([loginUser UTF8String] == g_pMainPanel->getSelfUserId()) {
            g_pMainPanel->onShowChatWnd(chatType, [xmppId UTF8String], [realJid UTF8String],
                                        [name UTF8String]);
        }
        else {

        }

        [center removeDeliveredNotification:notification];
    }
//    else if (notification.activationType == NSUserNotificationActivationTypeActionButtonClicked)
//    {
//        NSLog(@"NSUserNotificationActivationTypeActionButtonClicked");
//    }
    else if (notification.activationType == NSUserNotificationActivationTypeReplied)
    {
        const char* message = [[notification.response string] UTF8String];
        sendTextMessage(message, chatType, [xmppId UTF8String], [realJid UTF8String], [loginUser UTF8String]);
        [center removeDeliveredNotification:notification];
    }
}

+ (instancetype)instance {
    static MacNotification *notification = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        notification = [[self alloc] init];
        [notification initData];

    });
    return notification;
}

- (void)initData {

}

@end

namespace st
{
    namespace mac {
        void Notification::showNotification(StNotificationParam* param) {

            [[MacNotification instance] showNotification:param];
        }

    }
}

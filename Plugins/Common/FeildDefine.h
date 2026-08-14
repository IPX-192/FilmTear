#ifndef FEILDDEFNE_H
#define FEILDDEFNE_H


#define IN_PipeLineCleanGripDown                          "IN_PipeLineCleanGripDown"                          //流线清洗PCB夹爪在下
#define IN_PipeLineCleanGripUp                            "IN_PipeLineCleanGripUp"                            //流线清洗PCB夹爪在上
#define IN_PipeLineFeedTurntableGripDown                  "IN_PipeLineFeedTurntableGripDown"                  //流线转盘上料夹爪在下
#define IN_PipeLineFeedTurntableGripUp                    "IN_PipeLineFeedTurntableGripUp"                    //流线转盘上料夹爪在上
#define IN_PipeLineBlankTurntableGripDown                 "IN_PipeLineBlankTurntableGripDown"                 //流线转盘下料夹爪在下
#define IN_PipeLineBlankTurntableGripUp                   "IN_PipeLineBlankTurntableGripUp"                   //流线转盘下料夹爪在上
#define IN_TurntableCleanGripDown                         "IN_TurntableCleanGripDown"                         //转盘清洗PCB夹爪在下
#define IN_TurntableCleanGripUp                           "IN_TurntableCleanGripUp"                           //转盘清洗PCB夹爪在上
#define IN_FeedPCBGripOvervoltage                         "IN_FeedPCBGripOvervoltage"                         //上料PCB夹爪过压
#define IN_FeedHolderGripOvervoltage                      "IN_FeedHolderGripOvervoltage"                      //上料壳体夹爪过压
#define IN_FilmtearGripOvervoltage                        "IN_FilmtearGripOvervoltage"                        //撕膜夹爪过压


#define Out_PipeLineCleanGripDown                         "Out_PipeLineCleanGripDown"                         //流线清洗PCB夹爪向下
#define Out_PipeLineCleanGripUp                           "Out_PipeLineCleanGripUp"                           //流线清洗PCB夹爪向上
#define Out_PipeLineFeedTurntableGripUp                   "Out_PipeLineFeedTurntableGripUp"                   //流线转盘上料夹爪向上
#define Out_PipeLineFeedTurntableGripDown                 "Out_PipeLineFeedTurntableGripDown"                 //流线转盘上料夹爪向下
#define Out_PipeLineBlankTurntableGripUp                  "Out_PipeLineBlankTurntableGripUp"                  //流线转盘下料夹爪向上
#define Out_PipeLineBlankTurntableGripDown                "Out_PipeLineBlankTurntableGripDown"                //流线转盘下料夹爪向下
#define Out_TurntableCleanGripDown                        "Out_TurntableCleanGripDown"                        //转盘清洗PCB夹爪向下
#define Out_TurntableCleanGripUp                          "Out_TurntableCleanGripUp"                          //转盘清洗PCB夹爪向上


#define IN_FilmTearDetect                                 "IN_FilmTearDetect"                                 //撕膜色带检测


#define IN_FilmTearTest                                   "IN_FilmTearTest"                                   //测试使用


#define IN_PCBBoxTray                                     "IN_PCBBoxTray"                                     //PCB料仓有托盘
#define IN_HolderBoxTray                                  "IN_HolderBoxTray"                                  //壳体料仓有托盘
#define IN_HolderBoxReady                                 "IN_HolderBoxReady"                                 //前壳料仓就绪
#define IN_PCBBoxReady                                    "IN_PCBBoxReady"                                    //PCB料仓就绪
#define IN_PCBTrayTilt1                                   "IN_PCBTrayTilt1"                                   //PCB取料盘倾斜1
#define IN_PCBTrayTilt2                                   "IN_PCBTrayTilt2"                                   //PCB取料盘倾斜2
#define IN_HolderTrayTilt1                                "IN_HolderTrayTilt1"                                //壳体取料盘倾斜1
#define IN_HolderTrayTilt2                                "IN_HolderTrayTilt2"                                //壳体取料盘倾斜2


#define OUT_Test                                          "OUT_Test"                                          //测试使用


#define IN_PipeLinePCB                                    "IN_PipeLinePCB"                                    //流线上料PCB处有盘
#define IN_PipeLineClean                                  "IN_PipeLineClean"                                  //流线清洗PCB处有盘
#define IN_PipeLineTurntable                              "IN_PipeLineTurntable"                              //流线上料转盘处有盘
#define IN_PipeLineBackFlowBlockDown                      "IN_PipeLineBackFlowBlockDown"                      //回流线缓存阻挡在下
#define IN_PipeLineFeedPCBBlockDown                       "IN_PipeLineFeedPCBBlockDown"                       //流线上料PCB阻挡在下
#define IN_PipeLinePCBCleanBlockDown                      "IN_PipeLinePCBCleanBlockDown"                      //流线清洗PCB阻挡在下
#define IN_PipeLineTurntableBlockDown                     "IN_PipeLineTurntableBlockDown"                     //流线上料转盘阻挡在下
#define IN_PipeLineFeedPCBPushDown                        "IN_PipeLineFeedPCBPushDown"                        //流线上料PCB顶升在下
#define IN_PipeLineCleanPCBPushDown                       "IN_PipeLineCleanPCBPushDown"                       //流线清洗PCB顶升在下
#define IN_PipeLineTurntableFeedPushDown                  "IN_PipeLineTurntableFeedPushDown"                  //流线上料转盘顶升在下
#define IN_PipeLineFeedPCBTilt1                           "IN_PipeLineFeedPCBTilt1"                           //流水线上料PCB歪斜1
#define IN_PipeLineFeedPCBTilt2                           "IN_PipeLineFeedPCBTilt2"                           //流水线上料PCB歪斜2
#define IN_PipeLineCleanPCBTilt1                          "IN_PipeLineCleanPCBTilt1"                          //流水线清洗PCB歪斜1
#define IN_PipeLineCleanPCBTilt2                          "IN_PipeLineCleanPCBTilt2"                          //流水线清洗PCB歪斜2
#define IN_PipeLineTurntableFeedTilt1                     "IN_PipeLineTurntableFeedTilt1"                     //流水线上料转盘歪斜1
#define IN_PipeLineTurntableFeedTilt2                     "IN_PipeLineTurntableFeedTilt2"                     //流水线上料转盘歪斜2
#define IN_PipeLineBackFlowBuf                            "IN_PipeLineBackFlowBuf"                            //回流线缓存有盘


#define Out_PipeLineBackFlowMotorL                        "Out_PipeLineBackFlowMotorL"                        //回流线电机左转
#define Out_PipeLineBackFlowMotorR                        "Out_PipeLineBackFlowMotorR"                        //回流线电机右转
#define Out_PipeLineMotorL                                "Out_PipeLineMotorL"                                //输送线电机左转
#define Out_PipeLineMotorR                                "Out_PipeLineMotorR"                                //输送线电机右转
#define Out_PipeLineBackFlowBlockUp                       "Out_PipeLineBackFlowBlockUp"                       //回流线阻挡向上
#define Out_PipeLineFeedPCBBlockUp                        "Out_PipeLineFeedPCBBlockUp"                        //流线上料PCB阻挡上升
#define Out_PipeLinePCBCleanBlockUp                       "Out_PipeLinePCBCleanBlockUp"                       //流线清洗PCB阻挡上升
#define Out_PipeLineTurntableBlockUp                      "Out_PipeLineTurntableBlockUp"                      //流线上料转盘阻挡上升
#define Out_PipeLineFeedPCBPushUp                         "Out_PipeLineFeedPCBPushUp"                         //流线上料PCB顶升上升
#define Out_PipeLineFeedPCBPushDown                       "Out_PipeLineFeedPCBPushDown"                       //流线上料PCB顶升下降
#define Out_PipeLineCleanPCBPushDown                      "Out_PipeLineCleanPCBPushDown"                      //流线清洗PCB顶升下降
#define Out_PipeLineCleanPCBPushUp                        "Out_PipeLineCleanPCBPushUp"                        //流线清洗PCB顶升上升
#define Out_PipeLineTurntablePushDown                     "Out_PipeLineTurntablePushDown"                     //流线上料转盘顶升下降
#define Out_PipeLineTurntablePushUp                       "Out_PipeLineTurntablePushUp"                       //流线上料转盘顶升上升


#define IN_PipeLineTransferL                              "IN_PipeLineTransferL"                              //接驳线左边有盘
#define IN_PipeLineTransferR                              "IN_PipeLineTransferR"                              //接驳线右边有盘


#define Out_TransferBack                                  "Out_TransferBack"                                  //流线接驳台缩回
#define Out_TransferExend                                 "Out_TransferExend"                                 //流线接驳台伸出


#define In_Emg                                            "In_Emg"                                            //急停
#define In_Start1                                         "In_Start1"                                         //启动1
#define In_Start2                                         "In_Start2"                                         //启动2
#define In_Reset                                          "In_Reset"                                          //复位
#define In_Feed                                           "In_Feed"                                           //上料
#define In_AirCheck                                       "In_AirCheck"                                       //气压报警
#define In_SafeDoor                                       "In_SafeDoor"                                       //安全门
#define IN_Pause                                          "IN_Pause"                                          //暂停/恢复
#define IN_SafeGrate                                      "IN_SafeGrate"                                      //安全光栅
#define IN_TyphoonErr                                     "IN_TyphoonErr"                                     //台风系统异常
#define IN_Typhoon2                                       "IN_Typhoon2"                                       //台风系统备用2
#define IN_Typhoon4                                       "IN_Typhoon4"                                       //台风系统备用4
#define IN_Typhoon3                                       "IN_Typhoon3"                                       //台风系统备用3
#define IN_Typhoon5                                       "IN_Typhoon5"                                       //台风系统备用5


#define Out_Start1                                        "Out_Start1"                                        //启动灯1
#define Out_Start2                                        "Out_Start2"                                        //启动灯2
#define Out_Reset                                         "Out_Reset"                                         //复位灯
#define Out_RedLight                                      "Out_RedLight"                                      //报警红灯
#define Out_GreenLight                                    "Out_GreenLight"                                    //报警绿灯
#define Out_YellowLight                                   "Out_YellowLight"                                   //报警黄灯
#define Out_Buzzer                                        "Out_Buzzer"                                        //报警蜂鸣器
#define Out_LightBtn                                      "Out_LightBtn"                                      //照明按钮
#define Out_PipeLineBackFlowMotorPower                    "Out_PipeLineBackFlowMotorPower"                    //回流线电机电源
#define Out_PipeLineMotorPower                            "Out_PipeLineMotorPower"                            //输出线电机电源
#define Out_TyphoonAir                                    "Out_TyphoonAir"                                    //台风系统气源
#define Out_TyphoonStart                                  "Out_TyphoonStart"                                  //台风系统启动
#define Out_TyphooClearDirty                              "Out_TyphooClearDirty"                              //台风系统清灰
#define Out_Typhoon4                                      "Out_Typhoon4"                                      //台风系统备用输出1
#define Out_Typhoon5                                      "Out_Typhoon5"                                      //台风系统备用输出2


#define MotorPCBBoxY                                      "MotorPCBBoxY"                                      //PCB取料盘Y
#define MotorPCBBoxZ                                      "MotorPCBBoxZ"                                      //PCB料箱Z
#define MotorPCBGantryX                                   "MotorPCBGantryX"                                   //PCB龙门X轴
#define MotorPCBGantryY                                   "MotorPCBGantryY"                                   //PCB龙门Y轴
#define MotorPCBGantryZ                                   "MotorPCBGantryZ"                                   //PCB龙门Z轴
#define MotorPCBGripR                                     "MotorPCBGripR"                                     //PCB夹爪R
#define MotorPCBGripX                                     "MotorPCBGripX"                                     //PCB夹爪X
#define MotorHolderBoxY                                   "MotorHolderBoxY"                                   //壳体取料盘Y
#define MotorHolderBoxZ                                   "MotorHolderBoxZ"                                   //壳体料箱Z
#define MotorHolderGantryX                                "MotorHolderGantryX"                                //壳体龙门X轴
#define MotorHolderGantryY                                "MotorHolderGantryY"                                //壳体龙门Y轴
#define MotorHolderGantryZ                                "MotorHolderGantryZ"                                //壳体龙门Z轴
#define MotorHolderGripR                                  "MotorHolderGripR"                                  //壳体夹爪R
#define MotorHolderGripX                                  "MotorHolderGripX"                                  //壳体夹爪X
#define MotorTurntableBlankGripR                          "MotorTurntableBlankGripR"                          //转盘下料夹爪R
#define MotorTurntableBlankGripX                          "MotorTurntableBlankGripX"                          //转盘下料夹爪X
#define MotorTurntableFeedGripR                           "MotorTurntableFeedGripR"                           //转盘上料夹爪R
#define MotorTurntableFeedGripX                           "MotorTurntableFeedGripX"                           //转盘上料夹爪X
#define MotorTurntableGantryX                             "MotorTurntableGantryX"                             //转盘上下料X
#define MotorTurntableGantryY                             "MotorTurntableGantryY"                             //转盘上下料Y
#define MotorTurntableGantryZ                             "MotorTurntableGantryZ"                             //转盘上下料Z
#define MotorTurntableR                                   "MotorTurntableR"                                   //转盘旋转R
#define MotorFilmtearGantryY                              "MotorFilmtearGantryY"                              //撕膜龙门Y
#define MotorFilmtearGantryZ                              "MotorFilmtearGantryZ"                              //撕膜龙门Z
#define MotorFilmtearGripR                                "MotorFilmtearGripR"                                //撕膜搬运夹爪R
#define MotorFilmtearGripX                                "MotorFilmtearGripX"                                //撕膜搬运夹爪X
#define MotorFilmtearR1                                   "MotorFilmtearR1"                                   //撕膜旋转R1
#define MotorFilmtearR2                                   "MotorFilmtearR2"                                   //撕膜旋转R2
#define MotorFilmtearX                                    "MotorFilmtearX"                                    //撕膜机构X
#define MotorTransferX                                    "MotorTransferX"                                    //接驳料盘X
#define MotorTransferY                                    "MotorTransferY"                                    //接驳料盘Y
#define MotorCleanGripR                                   "MotorCleanGripR"                                   //流线清洗夹爪R
#define MotorCleanGripX                                   "MotorCleanGripX"                                   //流线清洗夹爪X
#define MotorPCBCleanX                                    "MotorPCBCleanX"                                    //流线翻转清洗X
#define MotorTurntableCleanGripR                          "MotorTurntableCleanGripR"                          //转盘清洗夹爪R
#define MotorTurntableCleanGripX                          "MotorTurntableCleanGripX"                          //转盘清洗夹爪X
#define MotorTurntableCleanX                              "MotorTurntableCleanX"                              //转盘翻转清洗X
#define MotorDirtyY                                       "MotorDirtyY"                                       //脏污检测Y


#define HolderGripOpen                                    "HolderGripOpen"                                    //壳体夹爪张开
#define HolderBoxCheckTray                                "HolderBoxCheckTray"                                //壳体料仓首层料盘检查位
#define HolderBoxGrabTray                                 "HolderBoxGrabTray"                                 //取料电机到料仓首层进入位
#define HolderTrayTouch                                   "HolderTrayTouch"                                   //首层料盘接触取料电机位
#define HolderBoxTrayUp                                   "HolderBoxTrayUp"                                   //首层料盘被取料电机托起位
#define HolderBoxMotorWait                                "HolderBoxMotorWait"                                //取料电机等待取料位
#define HolderScanCode                                    "HolderScanCode"                                    //壳体扫码
#define HolderPlace                                       "HolderPlace"                                       //壳体放置载具
#define PCBBoxCheckTray                                   "PCBBoxCheckTray"                                   //PCB料仓首层料盘检查位
#define PCBBoxGrabTray                                    "PCBBoxGrabTray"                                    //取料电机到料仓首层进入位
#define PCBTrayTouch                                      "PCBTrayTouch"                                      //首层料盘接触取料电机位
#define PCBBoxTrayUp                                      "PCBBoxTrayUp"                                      //首层料盘被取料电机托起位
#define PCBBoxMotorWait                                   "PCBBoxMotorWait"                                   //取料电机等待取料位
#define PCBScanCode                                       "PCBScanCode"                                       //PCB扫码
#define PCBPlace                                          "PCBPlace"                                          //PCB放置载具
#define PipeLineCleanPCB_Grab                             "PipeLineCleanPCB_Grab"                             //流线清洗PCB夹取
#define PipeLineCleanPCB_Clean                            "PipeLineCleanPCB_Clean"                            //流线清洗PCB清洗位
#define TurntableCleanPCB_Grab                            "TurntableCleanPCB_Grab"                            //转盘清洗PCB夹取
#define TurntableCleanPCB_Clean                           "TurntableCleanPCB_Clean"                           //转盘清洗PCB清洗位
#define TurntablePipeLineGrab                             "TurntablePipeLineGrab"                             //转盘流线上料抓取
#define TurntablePipeLinePlace                            "TurntablePipeLinePlace"                            //转盘流线下料放置
#define TurntableInitPos                                  "TurntableInitPos"                                  //转盘初始位
#define TurntableFeedPlace                                "TurntableFeedPlace"                                //转盘上料放置
#define TurntableBlankGrab                                "TurntableBlankGrab"                                //转盘下料抓取
#define FileTearFeed                                      "FileTearFeed"                                      //撕膜上料位
#define FileTearWork                                      "FileTearWork"                                      //撕膜工作位
#define FileTearDetect                                    "FileTearDetect"                                    //撕膜检测位
#define FileTearBlank                                     "FileTearBlank"                                     //撕膜下料位
#define HolderGripSafe                                    "HolderGripSafe"                                    //壳体夹爪安全位
#define PCBGripSafe                                       "PCBGripSafe"                                       //PCB夹爪安全位
#define TurntableFeedGripSafe                             "TurntableFeedGripSafe"                             //转盘上料龙门夹爪安全位
#define FilmTearGripSafe                                  "FilmTearGripSafe"                                  //撕膜夹爪安全位
#define DetectDirty                                       "DetectDirty"                                       //脏污检测位
#define HolderGripOpen                                    "HolderGripOpen"                                    //壳体夹爪张开
#define PCBGripOpen                                       "PCBGripOpen"                                       //PCB夹爪张开
#define PipeLinePCBCleanGripOpen                          "PipeLinePCBCleanGripOpen"                          //流线清洗PCB夹爪张开
#define TurntableFeedGripOpen                             "TurntableFeedGripOpen"                             //转盘上料夹爪张开
#define TurntableBlankGripOpen                            "TurntableBlankGripOpen"                            //转盘下料夹爪张开
#define FilmtearGripOpen                                  "FilmtearGripOpen"                                  //撕膜夹爪张开
#define TurntableCleanGripOpen                            "TurntableCleanGripOpen"                            //转盘清洗PCB夹爪张开
#define BackFlowTransfer                                  "BackFlowTransfer"                                  //回流线接驳位
#define FeedTransfer                                      "FeedTransfer"                                      //上料输送线接驳位


#endif // FEILDDEFNE_H
#ifndef MESHTTPPOST_H
#define MESHTTPPOST_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QMap>
#include <QTimer>
#include <QEventLoop>
#include <QDateTime>
#include <QFile>

// 工单工序
struct WorkProcess {
    QString processId;
    QString name;
};

// 用户认证返回的用户信息
struct AuthUserInfo
{
    QString deptName;
    QString updateDate;
    QString mobilePhone;
    QString phone;
    QString name;
    QString roleName;
    QString id;
    QString jobNumber;
    QString email;
    QString createDate;
    QString username;
};

// 生产信息单条字段
struct ProductionInfoItem
{
    QString name;
    QString value;
    QString key;
};

// 完整生产信息缓存
struct ProductionInfo
{
    QString pdPartNumber;   //产品料号
    QString pdName;         //产品名称
    QString pdCode;         //产品ERP
    QString bomVersion;     //BOM版本
    QString workOrderNum;   //工单号
    QString workOrderTotal; //工单总数
    QString batchNum;       //生产批次
    QString partNo;         //客户零件号
    QString processName;    //工序名称
    QString processSuccess; //合格数量
    QString processFail;    //失败数量
    QList<ProductionInfoItem> rawList; //原始数组备份
    ProductionInfo(){
        pdPartNumber.clear();
        pdName.clear();
        pdCode.clear();
        bomVersion.clear();
        workOrderNum.clear();
        workOrderTotal.clear();
        batchNum.clear();
        partNo.clear();
        processName.clear();
        processSuccess.clear();
        processFail.clear();
        rawList.clear();
    }
};

// 单条工序工作站信息
struct DeviceProcessItem
{
    QString workstationCode;
    QString workCenterCode;
    QString processId;
    QString processName;
    QString workstationName;
    QString deviceCode;
    QString workCenterName;
    QString deviceName;
};

// 工单
struct WorkOrder{
    QString workOrderId;
    QString name;
    QVector<WorkProcess> vecProcessId;
};

struct WorkOrderList {
    QVector<WorkOrder> vecWorkOrder;
};

// 产品/产线（登录工单查询缓存用）
struct PdList {
    QString id;
    QString name;
    WorkOrderList workOrderList;
};

struct PdLineList {
    QString id;
    QString name;
    QVector<PdList> vecPdList;
};

// 心跳上报报文
struct StationHeartBeat {
    QString stationId;
    QString workOrderId;
    QString processId;
    QString status;
    QDateTime beatTime;
};

// 上料
struct MaterialLoadInfo{
    QString type;           // 物料追溯类型，条码 ：1 批次 :2
    QString batchNumber;    // 物料批次
    int loadNum;            // 上料数量
//    int unitNum;            // 单个成品消耗物料数量（该参数选择性添加.如果传入则以传入值为准,不传入则以MES中BOM用量为准)
    QString materialInfoId; // 物料信息ID-必填
    QString erp;            // 物料编码-必填
    QString version;        // 物料版本-必填
    QString historyBatch;   // 原始批次
    QString mode;           // 生产模式编码备用
};

// 物料信息
struct MaterialInfo{
    QString id;
    QString name;
    QString code;
};

struct BindFixtureItem
{
    QString number;    // 探针条码
    QString channel;    //通道
};

struct FixtureConsumeItem
{
    QString number;    // 探针条码
    int consumption;   // 消耗值，0后端自动置1
};

// 治具寿命消耗单条
struct FixtureLifeItem
{
    QString number;
    qreal totalLife;
    qreal residueLife;
    int consumption;
};

struct FixtureConsumeResult
{
    bool warnFlag;
    QString warnMessage;
    QVector<FixtureLifeItem> fixtureList;
    FixtureConsumeResult(){
        warnFlag = false;
        warnMessage.clear();
        fixtureList.clear();
    }
};

struct DataDetail{
    QString message = "螺丝扭力";   // 例如
    QString name = "Torque1";
    QString result = "1";  // 1-成功  0-失败
    QString value = "0.5";
    QString lowerLimit = "";       // 非必填
    QString upperLimit = "";       // 非必填
    QString standard = "";          // 非必填
};

// 文件网关上传固定请求头结构体
struct GatewayUploadHeader
{
    QString user;        // 操作员工号
    QString userName;    // 操作人员姓名（未编码原始中文）
    QString appCode;     // 应用编码
    QString appSecretKey;// 应用密钥
};


// 文件上传返回
struct UploadFileResp {
    QString fileId;
    QString fullPath;
    QString filePId;
    QString createDirsRaw; // 目录数组原始json字符串
    QString contentMsg;    // content.message
    QString contentStatus;  // content.status
    QString version;
    UploadFileResp(){
        fileId.clear();
        fullPath.clear();
        filePId.clear();
        createDirsRaw.clear();
        contentMsg.clear();
        contentStatus.clear();
        version.clear();
    }
};

//接口枚举
enum ReplyStatus {
    replyNone = 0,
    replyQueryWorkOrderInfo,
    replyUserInfoAuth,
    replyConfigProcess,
    replyQueryProductionInfo,
    replyQueryProcessMaterial,
    replyMaterialLoad,
    replyValidateNumber,
    replySaveProcessOpResult,
    replyCompleteTask,
    replyQueryDeviceProcessInfo,
    replyStationHeartbeat,
    replyValidateDeviceUseFixture,
    replyBindFixtureChannel,
    replyConsumeFixtureLife,
    replyValidateStandardElementNumber,
    replyUploadSingle,
    replySaveProductFilePath
};

// 请求上下文
struct MesRequestContext {
    ReplyStatus reqType;
    QNetworkReply* reply;
    QJsonObject reqBody;
    QByteArray respData;
};

// 文件上传网关固定常量
const QString MES_UPLOAD_BOUNDARY = "----MES_UPLOAD_BOUNDARY_123456789";
const int UPLOAD_ERR_TRUNCATE_LEN = 200;
const int UPLOAD_JSON_ERR_TRUNCATE_LEN = 500;

const int   MES_NET_REQUEST_TIMEOUT_MS = 30000;

class MesHttpPost : public QObject
{
    Q_OBJECT
public:
    // 单例
    static MesHttpPost* Instance(QObject *parent = nullptr) {
        if (!m_instance)
            m_instance = new MesHttpPost(parent);
        return m_instance;
    }
    static void Uninstance();

protected:
    explicit MesHttpPost(QObject *parent = nullptr);
    ~MesHttpPost();

public:
    // 服务地址配置
    void SetMesIpInfo(QString url, QString deviceIp);
    QString GetMesBaseUrl(bool autoToken = false) const;
    QString GetStorageGatewayUrl() const;

    // 登录流程
    // 获取工单信息
    QString QueryWorkOrderInfo(QVector<PdLineList>& outPdLineList);
    // 用户信息认证
    QString UserInfoAuth(const QString& user, const QString& passwd, const QString& workOrderId, const QString& processId, AuthUserInfo& outUserInfo);
    // 设置工序
    QString ConfigProcess(const QString& processId, QString& outProcessKey);
    // 获取生产信息
    QString QueryProductionInfo(ProductionInfo& outProdInfo);
    // 获取指定工单下指定设备可做工序细信息
    QString QueryDeviceProcessInfo(const QString& deviceIp, const QString& workOrderId, QList<DeviceProcessItem>& outDeviceList);
    // 工作站与MES心跳交互
    QString StationHeartbeat();
    // 校验当前设备所做工序是否需要使用治具
    QString ValidateDeviceUseFixture(bool& outNeedFixture);
    // 查询治具剩余寿命&治具通道绑定校验接口
    QString BindFixtureChannel(const QList<BindFixtureItem>& bindList, FixtureConsumeResult& outResult);

    // 上料
    // 获取工序可用物料列表
    QString QueryProcessMaterial(QList<MaterialInfo>& outMaterialList);
    // 工作站与MES心跳交互
    // 上料接口
    QString MaterialLoad(const MaterialLoadInfo& info, bool& outLoad);

    // 自动生产流程
    // 治具消耗接口
    QString ConsumeFixtureLife(const QList<FixtureConsumeItem>& consumeList, FixtureConsumeResult& outResult);
    // 校验条码是否是标准件
    QString ValidateStandardElementNumber(const QString& sn,bool& outStandard);
    // 条码校验 sn-pcb条码  outValidate-校验结果
    QString ValidateNumber(const QString& sn,bool& outValidate);
    // 保存工序操作结果 sn-pcb条码  opResult-本站操作结果  detailAll-数据  outMainId-不管
    QString SaveProcessOpResult(const QString& sn, int opResult, const QList<DataDetail>& detailAll, QString& outMainId);
    // 工序过站接口(CompleteTask参数resultMainId是否使用待确定) sn-pcb条码  isSuccess-本站操作结果  errCode-错误码  bindMat-填true  outTaskResult-返回结果
    QString CompleteTask(const QString& sn, bool isSuccess, const QString& errCode, const QString& errInfo, bool bindMat, bool& outTaskResult);

    // ============================================================
    // 文件上传调用示例（按接口规范 filename 格式）：
    //
    //   GatewayUploadHeader header;
    //   header.user = "admin";                        // 操作员工号（固定值）
    //   header.userName = QString::fromUtf8("管理员"); // 中文名，内部自动 URL 编码
    //   header.appCode = "MES_APP";                   // 应用标识（固定值）
    //   header.appSecretKey = "MES_SECRET_2026";       // 应用凭证（固定值）
    //
    //   // filename 规范格式（每段自动 URL 编码）：
    //   //   车间(PB3)/产线(PEU2)/设备类型/YYMMDD/工单号/HHmmSS_条码.后缀
    //   QString fileName = "PB3/PEU2/检测设备/260728/WO20260702001/164530_SN123456.csv";
    //
    //   QFile file(localPath);
    //   file.open(QIODevice::ReadOnly);
    //   QByteArray fileBin = file.readAll();
    //   file.close();
    //
    //   UploadFileResp resp;
    //   QString err = UploadSingle(header, fileName, fileBin, "filePId值", "nameSpaceId值", resp);
    //   if (err.isEmpty()) { /* resp.fileId 可用 */ }
    //
    // ============================================================
    QString UploadSingle(const GatewayUploadHeader& headerInfo,const QString& fileName, const QByteArray& fileBin, const QString& filePid, const QString& nsId, UploadFileResp& outUploadResp);
    // 保存条码和文件路径关联关系
    QString SaveProductFilePath(const QString& sn, const QString& fileId, const QString& type, const QString& nsId, const QString& fileName,bool& outSave);

    // 解析函数错误信息存入静态map
    bool ReplyJsonFromQueryWorkOrderInfo(QJsonObject& jsonObject, QVector<PdLineList>& outPdLineList);
    bool ReplyJsonFromUserInfoAuth(QJsonObject& jsonObject, AuthUserInfo& outUserInfo);
    bool ReplyJsonFromConfigProcess(QJsonObject& jsonObject,QString& outProcessKey);
    bool ReplyJsonFromQueryProductionInfo(QJsonObject& jsonObject, ProductionInfo& outProdInfo);
    bool ReplyJsonFromQueryProcessMaterial(QJsonObject& jsonObject, QList<MaterialInfo>& outMaterialInfo);
    bool ReplyJsonFromMaterialLoad(QJsonObject& jsonObject, bool& outResult);
    bool ReplyJsonFromValidateNumber(QJsonObject& jsonObject,bool& outValidate);
    bool ReplyJsonFromSaveProcessOpResult(QJsonObject& jsonObject, QString& outMainId);
    bool ReplyJsonFromCompleteTask(QJsonObject& jsonObject, bool& outTaskResult);
    bool ReplyJsonFromQueryDeviceProcessInfo(QJsonObject& jsonObject, QList<DeviceProcessItem>& outDeviceList);
    bool ReplyJsonFromStationHeartbeat(QJsonObject& jsonObject);
    bool ReplyJsonFromValidateDeviceUseFixture(QJsonObject& jsonObject, bool& outNeedFixture);
    bool ReplyJsonFromBindFixtureChannel(QJsonObject & jsonObject, FixtureConsumeResult& outResult);
    bool ReplyJsonFromConsumeFixtureLife(QJsonObject& jsonObject, FixtureConsumeResult& outResult);       //注意：这里文档上出参"residuleLife"，应为"residueLife"
    bool ReplyJsonFromValidateStandardElementNumber(QJsonObject& jsonObject,bool& outStandard);
    bool ReplyJsonFromUploadSingle(QJsonObject& jsonObject, UploadFileResp& outUploadResp);
    bool ReplyJsonFromSaveProductFilePath(QJsonObject& jsonObject,bool& outSave);

public:
    // 有返回结构体，传引用
    template<typename T>
    QString SendMesPostRequest(ReplyStatus reqType, const QString& apiPath, const QJsonObject& body, T& outData)
    {
        // 内部调用私有void*底层，自动取地址
        return SendMesPostRequestImpl(reqType, apiPath, body, &outData);
    }

    // 无返回结构体（心跳、单纯校验接口）
    QString SendMesPostRequest(ReplyStatus reqType, const QString& apiPath, const QJsonObject& body)
    {
        // 传空指针
        return SendMesPostRequestImpl(reqType, apiPath, body, nullptr);
    }

protected:
    // JSON通用POST底层
    QString SendMesPostRequestImpl(ReplyStatus reqType, const QString& apiPath, const QJsonObject& body, void* outData);
    // 文件上传独立底层 multipart/form-data

    QString SendMultipartUploadRequest(const GatewayUploadHeader& headerInfo, const QString& fileName, const QByteArray& fileBin, const QString& filePid, const QString& nsId, UploadFileResp& outUploadResp);

    bool WaitRequestFinish(QNetworkReply* reply, int timeoutMs = MES_NET_REQUEST_TIMEOUT_MS);
    void SaveTestLog(const QByteArray& bytedata);
    QString EncodeWholePath(const QString& path);
    void HandleRequestError(QNetworkReply* reply, ReplyStatus reqType);
    void ClearRequestContext(QNetworkReply* reply);

protected:
    static MesHttpPost* m_instance;
    // 全局静态存储解析错误
    static QMap<QString, QString> m_parseErrMap;

    QString MesServerIp;
    QString MesServerPort;
	QString	MesUrl;
    QString m_iovtoken;
    QString m_processKey;
    QString m_ipInfo;

    QNetworkAccessManager m_httpPost;
    QMap<QNetworkReply*, MesRequestContext> m_reqMap;
    QMap<ReplyStatus, bool> m_reqRunning;

    QString m_processId;

public slots:
    void slotPostFinished(QNetworkReply* reply);
};

#endif // MESHTTPPOST

#ifndef CXI_HPP_
#define CXI_HPP_

#include <cstdint>
#include <fstream>
#include <vector>

enum class ContentFormType : uint8_t
{
    NOT_ASSIGNED = 0,
    SIMPLE_CONTENT = 1,
    EXECUTABLE_WITHOUT_ROMFS = 2,
    EXECUTABLE = 3
};

enum class ContentType : uint8_t
{
    UNSPECIFIED = 0,
    SYSTEM_UPDATE = 1,
    INSTRUCTION_MANUAL = 2,
    DOWNLOAD_PLAY_CHILD = 3,
    TRIAL = 4,
    EXTENDED_SYSTEM_UPDATE = 5,
};

enum class ContentPlateform : uint8_t
{
    NOT_ASSIGNED = 0,
    CTR = 1,
    SNAKE = 2
};

enum class SecurityFlags : uint8_t
{
    FIXED_CRYPTO_KEY = 0,
    NO_MOUNT_ROMFS = 2,
    NO_CRYPTO = 4,
    NEW_KEY_Y_GENERATOR = 0x20
};

struct CXIType {
    uint8_t types;

    ContentFormType getContentFormType() const
    {
        return static_cast<ContentFormType>(types & 0x1);
    }

    ContentType getContentType() const
    {
        return static_cast<ContentType>((types >> 1) & 0x7);
    }
};

struct NCCHFlags
{
    uint8_t reserved[3];
    uint8_t cryptoMethod;
    ContentPlateform platform;
    CXIType type;
    uint8_t contentUnitSize;
    SecurityFlags securityFlags;
};

struct NCCHHeader
{
    uint8_t signature[0x100];
    char magic[4];
    uint32_t contentSize;
    uint64_t partitionId;
    uint16_t makerCode;
    uint16_t version;
    uint32_t seedCheck;
    uint64_t programId;
    uint8_t reserved1[0x10];
    uint8_t logoHash[0x20];
    char productCode[0x10];
    uint8_t extendedHeaderHash[0x20];
    uint32_t extendedHeaderSize;
    uint8_t reserved2[0x4];
    NCCHFlags flags;
    uint32_t plainRegionOffset;
    uint32_t plainRegionSize;
    uint32_t logoRegionOffset;
    uint32_t logoRegionSize;
    uint32_t exefsOffset;
    uint32_t exefsSize;
    uint32_t exefsHashRegionSize;
    uint8_t reserved3[0x4];
    uint32_t romfsOffset;
    uint32_t romfsSize;
    uint8_t reserved4[0x4];
    uint8_t exefsSuperBlockHash[0x20];
    uint8_t romfsSuperBlockHash[0x20];
};

struct SystemControlFlags
{
    uint8_t flag;

    bool isCompressExefsCode() const
    {
        return flag & 0x1;
    }

    bool isSDApplication() const
    {
        return flag & 0x2;
    }
};

struct CodeSetInfo
{
    uint32_t address;
    uint32_t physicalRegionSize;
    uint32_t size;
};

struct SystemInfo
{
    uint64_t saveDataSize;
    uint64_t jumpId;
    uint8_t reserved[0x30];
};

struct SystemControlInfo
{
    char applicationTitle[0x8];
    uint8_t reserved1[0x5];
    SystemControlFlags flags;
    uint16_t remasterVersion;
    CodeSetInfo text;
    uint32_t stackSize;
    CodeSetInfo rodata;
    uint8_t reserved2[0x4];
    CodeSetInfo data;
    uint32_t bssSize;
    uint64_t dependencyList[48];
    SystemInfo systemInfo;
};

enum class CPUSpeed : uint8_t
{
    NORMAL = 0,
    HIGH = 1
};

struct PerformanceFlags
{
    uint8_t flag;

    bool isEnableL2Cache() const
    {
        return flag & 0x1;
    }

    CPUSpeed getCPUSpeed() const
    {
        return static_cast<CPUSpeed>((flag >> 1) & 0x1);
    }
};

enum class NSystemMode : uint8_t
{
    LEGACY = 0,
    PROD = 1,
    DEV1 = 2,
    DEV2 = 3,
    UNDEFINED = 4
};

struct NSystemModeFlags
{
    uint8_t flag;

    NSystemMode getSystemMode() const
    {
        uint8_t modeFlag = flag & 0xF;
        if (modeFlag <= 4)
            return static_cast<NSystemMode>(modeFlag);
        return NSystemMode::UNDEFINED;
    }
};

enum class SystemMode : uint8_t
{
    PROD = 0,
    UNDEFINED = 1,
    DEV1 = 2,
    DEV2 = 3,
    DEV3 = 4,
    DEV4 = 5
};

struct SystemModeFlags
{
    uint8_t flag;

    uint8_t getIdealProcessor() const
    {
        return flag & 0x3;
    }

    uint8_t getAffinityMask() const
    {
        return (flag >> 2) & 0x3;
    }

    SystemMode getSystemMode() const
    {
        uint8_t modeFlag = (flag >> 4) & 0xF;
        if (modeFlag <= 5)
            return static_cast<SystemMode>(modeFlag);
        return SystemMode::UNDEFINED;
    }
};

enum class OtherAttributes : uint8_t
{
    NOT_USE_ROMFS = 0,
    USE_EXTENDED_SAVE_DATA = 1,
};

enum class ResourceLimitCategory : uint8_t
{
    APPLICATION = 0,
    SYS_APPLET = 1,
    LIB_APPLET = 2,
    OTHER = 3
};

struct StorageInfo
{
    uint64_t extDataId;
    uint64_t saveDataId;
    uint64_t storageAccessibleIds;
    uint8_t fileSystemAccessInfo[0x7];
    OtherAttributes otherAttributes;
};

struct ARM11SystemCapabilities
{
    uint64_t programId;
    uint32_t coreVersion;
    PerformanceFlags performanceFlags;
    NSystemModeFlags nSystemModeFlags;
    SystemModeFlags systemModeFlags;
    uint8_t priority;
    uint8_t resourceLimitDescriptors[0x20];
    StorageInfo storageInfo;
    uint64_t serviceAccessControl[32];
    uint64_t extendedStorageAccessControl[2];
    uint8_t reserved[0xF];
    ResourceLimitCategory resourceLimitCategory;
};

struct ARM11KernelCapabilities
{
    uint32_t descriptors[28];
    uint8_t reserved[0x10];
};

struct ARM9AccessControl
{
    uint8_t descriptors[15];
    uint8_t descriptorVersion;
};

struct AccessControlInfo
{
    ARM11SystemCapabilities arm11SystemCapabilities;
    ARM11KernelCapabilities arm11KernelCapabilities;
    ARM9AccessControl arm9AccessControl;
};

struct ExtendedHeader
{
    SystemControlInfo systemControlInfo;
    AccessControlInfo accessControlInfo1;
    uint8_t accessDescSignature[0x100];
    uint8_t ncchHeaderRSAMod[0x100];
    AccessControlInfo accessControlInfo2;
};


struct FileHeader
{
    char filename[0x8];
    uint32_t offset;
    uint32_t size;
};

struct ExeFSHeader
{
    FileHeader fileHeaders[10];
    uint8_t reserved[0x20];
    uint8_t fileHashes[10][32];
};

struct CXIHeader
{
    NCCHHeader header;
    ExtendedHeader extendedHeader;
};

class CXI
{
    public:
        CXI(const std::string &path);
        ~CXI() = default;
        const CXIHeader& getHeader() const;
        std::vector<uint8_t> &getTextSection();
        std::vector<uint8_t> &getDataSection();
        std::vector<uint8_t> &getRodataSection();
    protected:
    private:
        CXIHeader _header;
        std::vector<uint8_t> _text;
        std::vector<uint8_t> _data;
        std::vector<uint8_t> _rodata;
        ExeFSHeader _exefsHeader;
};

#endif /* !CXI_HPP_ */

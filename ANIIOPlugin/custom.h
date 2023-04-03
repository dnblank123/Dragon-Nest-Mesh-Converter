/*--------------------自定义类型定义--------------------*/
//ANI文件头信息
struct AniHeader{
    char name[256];
    int version;
    int boneCount;  //总Bone数
    int animateCount;  //总动画数
};
//骨骼信息
struct BoneInfo{
	char boneName[256];	//骨骼名称
	char parentName[768];	//父骨骼名称
};
//动画变换数据
struct TransformationData{
    unsigned short frameIndex;
    Vec3F data;
};
struct RotationData{
    unsigned short frameIndex;
    Vec2F data;
};
struct ScalingData{
    unsigned short frameIndex;
    Vec3F data;
};
//动画基本变换数据
struct AnimateBaseData{
    Vec3F Transformation;
    Vec4F Rotation;
    Vec3F Scaling;
};

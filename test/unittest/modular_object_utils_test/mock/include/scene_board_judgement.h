#ifndef MOCK_SCENE_BOARD_JUDGEMENT_H
#define MOCK_SCENE_BOARD_JUDGEMENT_H

#include "mock_flag.h"

namespace OHOS {
namespace Rosen {
class SceneBoardJudgement {
public:
    static bool IsSceneBoardEnabled()
    {
        return MockFlag::isSceneBoardEnabled;
    }
};
} // namespace Rosen
} // namespace OHOS

#endif // MOCK_SCENE_BOARD_JUDGEMENT_H

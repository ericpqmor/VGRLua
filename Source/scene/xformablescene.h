#ifndef RVG_SCENE_XFORMABLESCENE_H
#define RVG_SCENE_XFORMABLESCENE_H

#include "scene/scene.h"
#include "xform/xformable.h"

namespace rvg {
    namespace scene {

// The input is imply a scene that can be xformed
class XformableScene: public rvg::xform::Xformable<XformableScene> {
private:
    using Scene = rvg::scene::Scene;
    using ScenePtr = rvg::scene::ScenePtr;
    ScenePtr m_scene_ptr;
public:
    XformableScene(const ScenePtr &scene_ptr): m_scene_ptr(scene_ptr) { ; }
    const Scene &scene(void) const { return *m_scene_ptr; }
    const ScenePtr &scene_ptr(void) const { return m_scene_ptr; }
};

using XformableScenePtr = std::shared_ptr<XformableScene>;

} } // namespace rvg::scene

#endif

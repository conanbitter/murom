#include "app.hpp"

namespace mrm {

void Scene::load() {
    loaded = true;
}

void Scene::tryLoad() {
    if (!loaded) load();
}

}  // namespace mrm
#include "Singleton.h"

SingletonManager g_singletonManagerInstance;

BaseSingleton* BaseSingleton::Store(std::unique_ptr<BaseSingleton>&& ptr)
{
	g_singletonManagerInstance.singletons_.push(std::move(ptr));
	return g_singletonManagerInstance.singletons_.top().get();
}

void BaseSingleton::Clear(BaseSingleton* clearPtr)
{
	std::stack<std::unique_ptr<BaseSingleton>> singletons;

	while (!g_singletonManagerInstance.singletons_.empty()) {
		auto ptr = std::move(g_singletonManagerInstance.singletons_.top());
		g_singletonManagerInstance.singletons_.pop();
		if (ptr.get() != clearPtr)
			singletons.push(std::move(ptr));
	}

	std::swap(singletons, g_singletonManagerInstance.singletons_);
}
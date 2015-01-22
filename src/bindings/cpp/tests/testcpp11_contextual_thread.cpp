#include <kdbthread.hpp>

#include <kdbprivate.h>

#include <gtest/gtest.h>

using namespace kdb;

void foo1(Coordinator & gc, KeySet & ks)
{
	Key specKey("/hello", KEY_CASCADING_NAME, KEY_END);

	ThreadContext c1(gc);
	ThreadValue<int> v1(ks, c1, specKey);
	ASSERT_EQ(v1, 8);

	v1 = 5;
	ASSERT_EQ(v1, 5);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	ASSERT_EQ(v1, 5);
}

void foo2(Coordinator & gc, KeySet & ks)
{
	Key specKey("/hello", KEY_CASCADING_NAME,  KEY_END);

	ThreadContext c2(gc);
	ThreadValue<int> v2(ks, c2, specKey);
	ASSERT_EQ(v2, 5);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	c2.update();
	ASSERT_EQ(v2, 5);

	v2 = 12;
	ASSERT_EQ(v2, 12);
}

TEST(test_contextual_thread, instanciation)
{
	Key specKey("/hello", KEY_CASCADING_NAME,  KEY_END);

	KeySet ks;
	ks.append(Key("user/hello", KEY_VALUE, "22", KEY_END));

	Coordinator gc;
	ThreadContext c(gc);
	ThreadValue<int> v(ks, c, specKey);
	ASSERT_EQ(v, 22);

	v = 8;
	ASSERT_EQ(v, 8);

	std::thread t1(foo1, std::ref(gc), std::ref(ks));

	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	c.update();
	ASSERT_EQ(v, 5);

	std::thread t2(foo2, std::ref(gc), std::ref(ks));
	t1.join();
	t2.join();

	c.update();
	ASSERT_EQ(v, 12);

	ks.append(Key("user/activate", KEY_VALUE, "88", KEY_END));
	v.activate();
	ASSERT_EQ(v, 88);
}

class Activate: public kdb::Layer
{
public:
	std::string id() const override
	{
		return "activate";
	}
	std::string operator()() const override
	{
		return "active";
	}
};

bool g_toggle = false;

void toggleOn()
{
	g_toggle = true;
}

void toggleOff()
{
	g_toggle = false;
}

void activate1(Coordinator & gc, KeySet & ks)
{
	Key specKey("/act/%activate%", KEY_CASCADING_NAME, KEY_END);

	ThreadContext c1(gc);
	ThreadValue<int> v1(ks, c1, specKey);
	c1.onLayerActivation<Activate>([](){toggleOn();});
	c1.onLayerDeactivation<Activate>([](){toggleOff();});
	ASSERT_EQ(v1, 10);
	c1.activate<Activate>();
	ASSERT_TRUE(g_toggle);
	ASSERT_EQ(v1, 22);
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	ASSERT_EQ(v1, 22);
	c1.deactivate<Activate>();
	ASSERT_FALSE(g_toggle);
	ASSERT_EQ(v1, 10);
}

TEST(test_contextual_thread, activate)
{
	Key specKey("/act/%activate%", KEY_CASCADING_NAME,  KEY_END);

	KeySet ks;
	ks.append(Key("user/act/%", KEY_VALUE, "10", KEY_END)); // not active layer
	ks.append(Key("user/act/active", KEY_VALUE, "22", KEY_END));

	Coordinator gc;
	ThreadContext c(gc);
	ThreadValue<int> v(ks, c, specKey);
	ASSERT_EQ(v, 10);

	std::thread t1(activate1, std::ref(gc), std::ref(ks));
	ASSERT_EQ(v, 10);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	ASSERT_TRUE(g_toggle);
	c.syncLayers();
	ASSERT_EQ(v, 22);
	t1.join();
	ASSERT_FALSE(g_toggle);
	ASSERT_EQ(v, 22);
	c.syncLayers();
	ASSERT_EQ(v, 10);
}

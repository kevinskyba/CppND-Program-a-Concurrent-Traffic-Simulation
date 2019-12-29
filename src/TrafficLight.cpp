#include <iostream>
#include <random>
#include <future>
#include <mutex>
#include "TrafficLight.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this] { return !_queue.empty(); });
    T t = std::move(_queue.back());
    _queue.pop_back();

    return t;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> uLock(_mutex);
    _queue.push_back(std::move(msg));
    _condition.notify_one();
}

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        TrafficLightPhase phase = _messageQueue.receive();
        if (phase == TrafficLightPhase::green) {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<int> distribution(4,6);
    auto start = std::chrono::high_resolution_clock::now();
    auto duration = distribution(eng);
    while(true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start;
        if (elapsed.count() >= duration) {
            _currentPhase = (_currentPhase == TrafficLightPhase::green ? TrafficLightPhase::red : TrafficLightPhase::green);
            _messageQueue.send(std::move(_currentPhase));
            // Set new duration for next cycle
            duration = distribution(eng);
            // Reset the timer
            start = std::chrono::high_resolution_clock::now();
        }
    }
}

#pragma clang diagnostic pop
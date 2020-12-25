#include <iostream>
#include <random>
#include <thread>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

//generic message queue using templates: source is udacity Concurrency: lesson 4, concept 3 (building a concurrent message).
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
   // perform queue modification under the lock
   std::unique_lock<std::mutex> uLock(_mutex);
   _cond.wait(uLock, [this] { return !_queue.empty(); }); // pass unique lock to condition variable

   // remove last vector element from queue
   T msg = std::move(_queue.back());
   _queue.pop_back();

   return msg; // will not be copied due to return value optimization (RVO) in C++
}
template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
   // simulate some work
   std::this_thread::sleep_for(std::chrono::milliseconds(100));

   // perform vector modification under the lock
   std::lock_guard<std::mutex> uLock(_mutex);

   // add msg to queue
  //std::cout << "   Message " << msg << " has been sent to the queue" << std::endl;
   _queue.push_back(std::move(msg));
   _cond.notify_one(); // notify client after pushing new T type object
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}
void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  while(true){
    TrafficLightPhase color = _messageQueue.receive();
    if (color==TrafficLightPhase::green){
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
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  	//starts function cycleThroughPhases of the current instance of the TrafficLight object and adds its reference to threads so that in destructor, all threads can be joined.
	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this)); //emplace_back instead of push_back to "avoid" creating new object and copying
}
// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
  
    // using the help from the answer: https://knowledge.udacity.com/questions/96814 Extract from Vehicle.cpp
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_real_distribution<> distr(4000, 6000);
    auto cycle_duration = distr(eng);
	
  	// start time counter, https://en.cppreference.com/w/cpp/chrono/time_point
  	// using system_clock instead of high_resolution_clock
    std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();
  
    while (true)
    {
		//wait as requested	
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      
      	// difference  between current and start time using milliseconds cast.
        auto time_diff = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count();

        if (time_diff >= cycle_duration)
          {      
              if (_currentPhase == TrafficLightPhase::red) {
                  _currentPhase =  TrafficLightPhase::green; } 
              else { _currentPhase = TrafficLightPhase::red;  }

              _messageQueue.send(std::move(_currentPhase)); // send using move semantics for sending as r value referece to avoid copying
              time_diff = 0; // reset timer
              start_time = std::chrono::system_clock::now(); // reset the start time.
          	  cycle_duration = distr(eng); // reset cycle duration
			}
   	 }
}



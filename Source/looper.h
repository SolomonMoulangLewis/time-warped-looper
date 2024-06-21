#pragma once
#include <algorithm>
#include "dsp.h"

class Looper
{
public:
    /**
           @brief A looper with the ability to:
                - Divide recorded loop into smaller segments 2, ..., 128
                - Select currently looped segment
                - Select Playback State: reverse, half-time, double-time playback modes
                - Record in any playback state: reverse, half-time, double time
           @author Solomon Moulang Lewis
           @date Jun 2024
    */
    Looper(){}
    ~Looper(){}
    
    enum {
        LISTENING,
        PLAYING,
        RECORDING
    } state_ = LISTENING;
    
    enum {
        NORMAL,
        REVERSE,
        HALF_SPEED,
        DOUBLE_SPEED
    } time_manipulation_state_ = NORMAL;
    
    void Init (float *mem, size_t size)
    {
        buff_ = mem;
        buffer_size_ = size;
        
        InitBuff();
    }
    
    float Process (const float input)
    {
        float sig = 0.f;
        float inc = GetIncrementSize();
        
        switch (state_)
        {
            case LISTENING:
                sig = input;
                break;
            //====================================================
            case RECORDING:
                //
                sig = input; // when recording only listen to input
                Write( pos_, input);
                
                // reset recsize_ and set recsize_reset_ flag
                if (!recsize_reset_)
                {
                    recsize_ = 0;
                    recsize_reset_ = true;
                }
                
                pos_ += inc;
                recsize_ += fabsf (inc);
                
                WrapPosToBuffer();
                
                // ensure max recsize_ == buffer_size_
                if (recsize_ >= buffer_size_)
                {
                    recsize_ = buffer_size_;
                }
                
                loop_size_ = recsize_;
                
                loop_reset_ = false;
                break;
            //====================================================
            case PLAYING:
                //
                if (!loop_reset_)
                {
                    // calculate loop_start_pos_ position in buffer
                    if (inc > 0)
                        loop_start_pos_ = pos_ - recsize_; // forward record
                    else
                        loop_start_pos_ = pos_ + recsize_ - 1; // reverse record
                    
                    recorded_in_reverse_ = inc < 0 ? true : false;
                    
                    // wrap loop_start_pos_ to buffer
                    if (loop_start_pos_ >= buffer_size_)
                        loop_start_pos_ -= buffer_size_;
                    else if (loop_start_pos_ < 0)
                        loop_start_pos_ += buffer_size_;
                    
                    loop_end_pos_ = pos_;
                    
                    // update pos
                    pos_ = loop_start_pos_;
                    
                    loop_reset_ = true;
                }
                
                sig = ReadF (pos_); // read with interpolation
                pos_ += inc;
                
                WrapPosToSegments (inc);
                
                WrapPosToBuffer();
                
                // reset flag for ensuring new recording size when entering playback state
                recsize_reset_ = false;
                break;
        }
        return sig;
    }
    
    void UpdatePlaybackState()
    {
        switch (state_)
        {
            case LISTENING:
                state_ = RECORDING;
                std::cout << "state RECORDING" << std::endl;
                break;
            case RECORDING:
                state_ = PLAYING;
                std::cout << "state PLAYING" << std::endl;
                break;
            case PLAYING:
                state_ = LISTENING;
                std::cout << "state LISTENING" << std::endl;
                break;
        };
    }
    
    void SetTimeManipulation (float param)
    {
        if (param > 0.75f)
        {
            time_manipulation_state_ = DOUBLE_SPEED;
        }
        else if (param > 0.5f)
        {
            time_manipulation_state_ = HALF_SPEED;
        }
        else if (param > 0.25f)
        {
            time_manipulation_state_ = REVERSE;
        }
        else
        {
            time_manipulation_state_ = NORMAL;
        }
    }
    
    void SetSegmentDivisions (float loop_length_param)
    {
        if (loop_length_param <= 0.125f)
            loop_size_ = recsize_;
        else if (loop_length_param <= 0.25f)
            loop_size_ = recsize_ / 2; //               /2
        else if (loop_length_param <= 0.375f)
            loop_size_ = recsize_ / 4; //               /4
        else if (loop_length_param <= 0.5f)
            loop_size_ = recsize_ / 8; //               /8
        else if (loop_length_param <= 0.625f)
            loop_size_ = recsize_ / 16; //              /16
        else if (loop_length_param <= 0.75f)
            loop_size_ = recsize_ / 32; //              /32
        else if (loop_length_param <= 0.875f)
            loop_size_ = recsize_ / 64; //              /64
        else
            loop_size_ = recsize_ / 128; //             /128
    }
    
    void SetSelectedSegment (float param)
    {
        selected_segment_ = param;
    }
    
private:
    void InitBuff() { std::fill (&buff_[0], &buff_[buffer_size_ - 1], 0); }
    
    inline const float Read (size_t pos) const { return buff_[pos]; }
    float ReadF(float pos)
    {
        float    a, b, frac;
        uint32_t i_idx = static_cast<uint32_t>(pos);
        frac           = pos - i_idx;
        a              = buff_[i_idx];
        b              = buff_[(i_idx + 1) % buffer_size_];
        return a + (b - a) * frac;
    }
    inline void Write (size_t pos, float val) { buff_[pos] = val; }
    
    float GetIncrementSize()
    {
        switch (time_manipulation_state_)
        {
            case NORMAL:
                return 1.0f;
            case REVERSE:
                return -1.0f;
            case HALF_SPEED:
                return 0.5f;
            case DOUBLE_SPEED:
                return 2.0f;
        }
        return 1.0f;
    }
    
    void WrapPosToBuffer()
    {
        if (pos_ >= buffer_size_)
            pos_ -= buffer_size_;
        else if (pos_ < 0)
            pos_ += buffer_size_;
    }
    
    // TODO: fix clicks at loop points
    void WrapPosToSegments (float &inc)
    {
        // if recorded in reverse then loop from end -> start
        // else start -> end
        float start_pos = recorded_in_reverse_ ? loop_end_pos_ : loop_start_pos_;
        float end_pos = recorded_in_reverse_ ? loop_start_pos_ : loop_end_pos_;
        bool forward = inc > 0;
        
        int num_segments = static_cast<int> (recsize_ / loop_size_);
        int current_segment = static_cast<int>(selected_segment_ * num_segments);
        if (current_segment >= num_segments) current_segment = num_segments - 1;
        if (current_segment < 0) current_segment = 0;

        if (recorded_in_reverse_)
        {
            if (forward)
            {
                start_pos += (current_segment * loop_size_);
                end_pos = start_pos + loop_size_;
            }
            else
            {
                end_pos -= (current_segment * loop_size_);
                start_pos = end_pos - loop_size_;
            }
        }
        else
        {
            if (forward)
            {
                start_pos += (current_segment * loop_size_);
                end_pos = start_pos + loop_size_;
            }
            else
            {
                end_pos -= (current_segment * loop_size_);
                start_pos = end_pos - loop_size_;
            }
        }

        // wrap segment positions to buffer size
        if (start_pos < 0)
        {
            start_pos += buffer_size_;
        }
        else if (start_pos >= buffer_size_)
        {
            start_pos -= buffer_size_;
        }

        if (end_pos < 0)
        {
            end_pos += buffer_size_;
        }
        else if (end_pos >= buffer_size_)
        {
            end_pos -= buffer_size_;
        }

        // wrap pos to start/end pos
        if (start_pos < end_pos)
        {
            if ((forward && (pos_ < start_pos || pos_ > end_pos)) ||
                (!forward && (pos_ > end_pos || pos_ < start_pos)))
            {
                // entered if recorded in normal

                pos_ = forward ? start_pos : end_pos;
            }
        }
        else
        {
            if ((forward && (pos_ < start_pos && pos_ > end_pos)) ||
                (!forward && (pos_ > end_pos && pos_ < start_pos)))
            {
                // entered if recorded in reverse

                pos_ = forward ? start_pos : end_pos;
            }
        }
    }
    
    size_t buffer_size_ = 0;
    float *buff_ = nullptr;
    
    float pos_ = 0;
    
    float inc_ = 0;
    
    bool recsize_reset_ = false;
    
    bool recorded_in_reverse_ = false;
    
    float selected_segment_ = 0.f;
    int num_segments_;
    
    float recsize_ = 0;
    size_t loop_size_ = 0;
    float loop_start_pos_ = 0;
    float loop_end_pos_ = 0;
    bool loop_reset_ = false;
    
    float fade_samples = 10.f;
};

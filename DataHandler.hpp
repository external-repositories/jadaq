/**
 * jadaq (Just Another DAQ)
 * Copyright (C) 2018  Troels Blum <troels@blum.dk>
 *
 * @author Troels Blum <troels@blum.dk>
 * @section LICENSE
 * This program is free software: you can redistribute it and/or modify
 *        it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 *         but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @section DESCRIPTION
 * Interface for the data handlers
 *
 */

#ifndef JADAQ_DATAHANDLER_HPP
#define JADAQ_DATAHANDLER_HPP

#include <functional>
#include "DataFormat.hpp"
#include "uuid.hpp"
#include "EventAccessor.hpp"
#include "EventIterator.hpp"
#include "container.hpp"
#include "DataWriter.hpp"


class DataHandlerGeneric
{
public:
    static int64_t getTimeMsecs()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
    }

};

/*
 * E is element type e.g. Data::ListElementxxx
 * C is containertype i.e. jadaq::vector, jadaq::set, jadaq::buffer
 */
template <typename E, template<typename...> typename C>
class DataHandler
{
    static_assert(std::is_pod<E>::value, "E must be POD");
private:
    DataWriter* dataWriter;
    uint32_t digitizerID;
    size_t numChannels;
    struct Buffer
    {
        C<E>* buffer;
        typename E::time_t* maxLocalTime; // Array containing MaxLocalTime from the previous insertion needed to detect reset
        uint64_t globalTimeStamp = 0;
        void clear(size_t numChannels)
        {
            buffer->clear();
            for (size_t i = 0; i < numChannels; ++i)
            {
                maxLocalTime[i] = 0;
            }
            globalTimeStamp = 0;
        }
    } current, next;
public:
    DataHandler( DataWriter* dw, uint32_t digID, size_t channels)
            : dataWriter(dw)
            , digitizerID(digID)
            , numChannels(channels)
    {
        current.buffer = new C<E>();
        current.maxLocalTime = new typename E::time_t[numChannels];
        current.clear(numChannels);
        next.buffer = new C<E>();
        next.maxLocalTime = new typename E::time_t[numChannels];
        next.clear(numChannels);
    }
    size_t operator()(EventIterator& it)
    {
        DPPQDCEventIterator<E>& eventIterator = it.base<E>();
        size_t events = 0;
        for (;eventIterator != eventIterator.end(); ++eventIterator)
        {
            events += 1;
            E element = *eventIterator;
            if (element.localTime > current.maxLocalTime[element.channel])
            {
                current.maxLocalTime[element.channel] = element.localTime;
                try {
                    current.buffer->insert(element);
                } catch (std::length_error&)
                {
                    (*dataWriter)(current.buffer,digitizerID,current.globalTimeStamp);
                    current.buffer->clear();
                    current.buffer->insert(element);
                }
            } else {
                next.maxLocalTime[element.channel] = element.localTime;
                if (next.globalTimeStamp == 0)
                {
                    next.globalTimeStamp = DataHandlerGeneric::getTimeMsecs();
                }
                try {
                    next.buffer->insert(element);
                } catch (std::length_error&)
                {
                    (*dataWriter)(next.buffer,digitizerID,next.globalTimeStamp);
                    next.buffer->clear();
                    next.buffer->insert(element);
                }
            }
        }
        if (!next.buffer->empty())
        {
            (*dataWriter)(current.buffer,digitizerID,current.globalTimeStamp);
            current.clear(numChannels);
            Buffer temp = current;
            current = next;
            next = temp;
        }
        return events;
    }
    void flush()
    {
        (*dataWriter)(current.buffer,digitizerID,current.globalTimeStamp);
        current.clear(numChannels);
        (*dataWriter)(next.buffer,digitizerID,next.globalTimeStamp);
        next.clear(numChannels);
    }
};
#endif //JADAQ_DATAHANDLER_HPP

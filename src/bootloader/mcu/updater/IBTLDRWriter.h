/*

Copyright 2015-2020 Igor Petrovic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#pragma once

#include <inttypes.h>

class IBTLDRWriter
{
    public:
    virtual size_t pageSize(size_t index)                    = 0;
    virtual void   erasePage(uint32_t address)               = 0;
    virtual void   fillPage(uint32_t address, uint16_t data) = 0;
    virtual void   writePage(uint32_t address)               = 0;
    virtual void   apply()                                   = 0;
};
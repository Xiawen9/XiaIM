#pragma once
#include <map>
#include <string>

#include "c2s_message.h"
#include "result_code.h"
#include "s2c_message.h"

extern std::map<int, int> g_Client;  // client Ψһ, �� userid ����

//���ݿ���
std::string g_strDBName = "TestIM";

#include "silero_vad.h"
#include <streambuf>
#include <istream>
#include <Windows.h>
#include "resource.h"

std::shared_ptr<torch::jit::Module> SileroVAD::m_pModel;
bool SileroVAD::m_bInited = false;

std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);

    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);

    return message;
}

std::istringstream load_model_weights(){
    HMODULE hLib = LoadLibrary(TEXT("resource.dll"));
    std::istringstream isstream("");
    if (hLib == NULL) {
        std::stringstream ss;
        ss << "Cannot load dll: " <<  GetLastErrorAsString();
        throw std::runtime_error(ss.str());
    }
    HRSRC hRes = FindResource(hLib, MAKEINTRESOURCE(IDR_SILERO_VAD), TEXT("RT_WEIGHT"));
    if (hRes == NULL) {
        std::stringstream ss;
        ss << "Cannot find resource: " <<  GetLastErrorAsString();
        throw std::runtime_error(ss.str());
    }

    HGLOBAL hResData = LoadResource(hLib, hRes);
    if (hResData == NULL) {
        std::stringstream ss;
        ss << "Cannot load resource: " <<  GetLastErrorAsString();
        throw std::runtime_error(ss.str());
    }

    void* pRes = LockResource(hResData);
    if (pRes == NULL) {
        std::stringstream ss;
        ss << "Cannot access resource: " <<  GetLastErrorAsString();
        throw std::runtime_error(ss.str());
    }

    DWORD resSize = SizeofResource(hLib, hRes);
    std::string binary_string((char*)pRes, resSize);
    return std::istringstream(binary_string, std::ios::binary | std::ios::in);
}

void SileroVAD::init_model()
{
    if (m_pModel == nullptr)
    {
        try
        {
            // Create istream from memory
            std::istringstream input_stream = load_model_weights();

            // Load the TorchScript model from stream
            m_pModel = std::make_shared<torch::jit::Module>(torch::jit::load(input_stream));
            m_pModel->eval();

            m_bInited = true;
        }
        catch (const c10::Error &e)
        {
            std::stringstream ss;
            ss << "Error loading the model: " << e.what();
            throw std::runtime_error(ss.str());
            return;
        }
    }
}

void SileroVAD::reset_states()
{
    if (!m_bInited)
        return;
    m_pModel->run_method("reset_states");
}

float SileroVAD::predict(const std::vector<float> &chunk, int sr)
{
    auto x = torch::tensor(chunk, torch::kFloat);

    return predict(x, sr);
}

float SileroVAD::predict(const torch::Tensor &chunk, int sr)
{
    if (!m_bInited)
        init_model();
    return m_pModel->forward({chunk, torch::Scalar(sr)}).toTensor().item().toFloat();
}

std::string SileroVAD::dump_model_to_string(bool print_method_bodies, bool print_attr_values, bool print_param_values)
{
    if (!m_bInited)
        return "";

    return m_pModel->dump_to_str(print_method_bodies, print_attr_values, print_param_values);
}

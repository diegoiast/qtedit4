#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include <lsp/connection.h>
#include <lsp/io/stream.h>
#include <lsp/messagehandler.h>
#include <lsp/messages.h>
#include <lsp/process.h>

class LspClientImpl {
  public:
    explicit LspClientImpl();
    ~LspClientImpl();

    // Disable copy and move
    LspClientImpl(const LspClientImpl &) = delete;
    LspClientImpl &operator=(const LspClientImpl &) = delete;
    LspClientImpl(LspClientImpl &&) = delete;
    LspClientImpl &operator=(LspClientImpl &&) = delete;

    void debugIO(bool enable);

    void setDocumentRoot(const std::string &documentRoot);
    void openDocument(const std::string &fileName, const std::string &fileContents);
    void hover(const std::string &fileName, int line, int column,
               std::function<void(lsp::requests::TextDocument_Hover::Result &&result)> callback);
    void references(const std::string &fileName, int line, int column,
                    std::function<void(lsp::requests::TextDocument_References::Result &&)> callback);
    void definition(const std::string &fileName, int line, int column,
                    std::function<void(lsp::requests::TextDocument_Definition::Result &&)> callback);
    void implementation(const std::string &fileName, int line, int column,
                        std::function<void(lsp::requests::TextDocument_Implementation::Result &&)> callback);

    void startClangd();
    void stopClangd();
    void initializeLspServer();
    void shutdownLspServer();
    void setOnServerInitialized(
        std::function<void(lsp::requests::Initialize::Result &&result)> callback);

  private:
    std::function<void(lsp::requests::Initialize::Result &&result)> m_onServerInitialized;
    void runLoop();
    std::string m_documentRoot;
    std::unique_ptr<lsp::Connection> m_connection;
    std::unique_ptr<lsp::MessageHandler> m_messageHandler;
    std::unique_ptr<lsp::Process> m_clandIO;

    std::thread m_workerThread;
    std::atomic_bool m_running{false};
};

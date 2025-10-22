#include <exception>
#include <iostream>
#include <utility>

#include "LspClientImpl.hpp"
#include "lsp/fileuri.h"
#include "lsp/messages.h"
#include "lsp/types.h"

LspClientImpl::LspClientImpl() {}

void LspClientImpl::debugIO(bool enable) { (void)enable; }

void LspClientImpl::setDocumentRoot(const std::string &newRoot) {
    m_documentRoot = newRoot;
    initializeLspServer();
}

void LspClientImpl::openDocument(const std::string &fileName, const std::string &fileContents) {
    if (!m_running) {
        return;
    }
    // When opening a file:
    lsp::notifications::TextDocument_DidOpen::Params params{
        .textDocument = {
            .uri = lsp::FileUri::fromPath(fileName),
            .languageId = "cpp", // or "c", "python", etc.
            .version = 1,
            .text = fileContents // The full text of the opened file
        }};
    m_messageHandler->sendNotification<lsp::notifications::TextDocument_DidOpen>(std::move(params));
}

void LspClientImpl::hover(
    const std::string &fileName, int line, int column,
    std::function<void(lsp::requests::TextDocument_Hover::Result &&result)> callback) {

    if (!m_running) {
        return;
    }

    lsp::HoverParams params;
    params.textDocument.uri = lsp::FileUri::fromPath(fileName);
    params.position.line = line;
    params.position.character = column;
    // params.workDoneToken

    m_messageHandler->sendRequest<lsp::requests::TextDocument_Hover>(
        std::move(params),
        [callback = std::move(callback)](auto result) { callback(std::move(result)); },
        [](const lsp::Error &error) {
            std::cerr << "Failed to get response from LSP server: " << error.what() << std::endl;
        });
}

void LspClientImpl::references(
    const std::string &fileName, int line, int column,
    std::function<void(lsp::requests::TextDocument_References::Result &&result)> callback) {
    if (!m_running) {
        return;
    }

    lsp::ReferenceParams params;
    params.textDocument.uri = lsp::FileUri::fromPath(fileName);
    params.position.line = line;
    params.position.character = column;
    params.context.includeDeclaration = true;

    m_messageHandler->sendRequest<lsp::requests::TextDocument_References>(
        std::move(params),
        [callback = std::move(callback)](auto result) { callback(std::move(result)); },
        [](const lsp::Error &error) {
            std::cerr << "Failed to get references from LSP server: " << error.what() << std::endl;
        });
}

void LspClientImpl::definition(
    const std::string &fileName, int line, int column,
    std::function<void(lsp::requests::TextDocument_Definition::Result &&)> callback) {
    if (!m_running) {
        return;
    }

    lsp::DefinitionParams params;
    params.textDocument.uri = lsp::FileUri::fromPath(fileName);
    params.position.line = line;
    params.position.character = column;

    m_messageHandler->sendRequest<lsp::requests::TextDocument_Definition>(
        std::move(params),
        [callback = std::move(callback)](auto result) { callback(std::move(result)); },
        [](const lsp::Error &error) {
            std::cerr << "Failed to get definition from LSP server: " << error.what() << std::endl;
        });
}

void LspClientImpl::implementation(
    const std::string &fileName, int line, int column,
    std::function<void(lsp::requests::TextDocument_Implementation::Result &&)> callback) {
    if (!m_running) {
        return;
    }

    lsp::ImplementationParams params;
    params.textDocument.uri = lsp::FileUri::fromPath(fileName);
    params.position.line = line;
    params.position.character = column;

    m_messageHandler->sendRequest<lsp::requests::TextDocument_Implementation>(
        std::move(params),
        [callback = std::move(callback)](auto result) { callback(std::move(result)); },
        [](const lsp::Error &error) {
            std::cerr << "Failed to get implementation from LSP server: " << error.what() << std::endl;
        });
}

LspClientImpl::~LspClientImpl() {
    stopClangd();
}

void LspClientImpl::startClangd() {
    try {
#if defined(WIN32)
        m_clandIO = std::make_unique<lsp::Process>("C:\\Program Files\\LLVM\\bin\\clangd.exe");
#elif defined(__unix__)
        m_clandIO = std::make_unique<lsp::Process>("/usr/bin/clangd");
#endif
        m_connection = std::make_unique<lsp::Connection>(m_clandIO->stdIO());
        m_messageHandler = std::make_unique<lsp::MessageHandler>(*m_connection);
        m_running = true;
        m_workerThread = std::thread(&LspClientImpl::runLoop, this);
    } catch (lsp::ProcessError e) {
    }
}

void LspClientImpl::stopClangd() {
    if (m_running) {
        shutdownLspServer();
    }

    if (m_clandIO) {
        m_clandIO->wait();
    }
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
}

void LspClientImpl::setOnServerInitialized(
    std::function<void(lsp::requests::Initialize::Result &&result)> callback) {
    m_onServerInitialized = std::move(callback);
}

void LspClientImpl::initializeLspServer() {
    if (!m_running) {
        return;
    }

    auto initializeParams = lsp::requests::Initialize::Params{};
    initializeParams.rootUri = lsp::FileUri::fromPath(m_documentRoot);
    initializeParams.capabilities = {};

    m_messageHandler->sendRequest<lsp::requests::Initialize>(
        std::move(initializeParams),
        [this](lsp::requests::Initialize::Result &&result) {
            if (m_onServerInitialized) {
                m_onServerInitialized(std::move(result));
            }
        },
        [](const lsp::Error &error) {
            std::cerr << "Failed to get response from LSP server: " << error.what() << std::endl;
        });

#if 0
    auto [id, result] = m_messageHandler->sendRequest<lsp::requests::TextDocument_Diagnostic>(
        lsp::requests::TextDocument_Diagnostic::Params{/* parameters */});
#endif
}

void LspClientImpl::shutdownLspServer() {
    if (!m_running) {
        return;
    }

    m_messageHandler->sendRequest<lsp::requests::Shutdown>(
        [this](lsp::json::Null) {
            m_messageHandler->sendNotification<lsp::notifications::Exit>();
            m_running = false;
        },
        [this](const lsp::Error &) {
            m_messageHandler->sendNotification<lsp::notifications::Exit>();
            m_running = false;
        });
}

void LspClientImpl::runLoop() {
    while (m_running) {
        try {
            m_messageHandler->processIncomingMessages();
        } catch (const lsp::ConnectionError &e) {
            break;
        } catch (const std::exception &e) {
            std::cerr << "Exception in runLoop: " << e.what() << std::endl;
        }
    }
}

#include "WE_RenderCore.hpp"
#include <utility> // for std::swap

#if WE_DUAL_CORE_RENDER

void Renderer::displayTask_wrapper(void* param) {
    static_cast<Renderer*>(param)->displayTask_impl();
}

void Renderer::displayTask_impl() {
    while (true) {
        xSemaphoreTake(m_renderReady, portMAX_DELAY);
        if (m_displayTaskShouldExit) break;

        int tmp        = m_frontBufIdx;
        m_frontBufIdx  = m_backBufIdx;
        m_backBufIdx   = tmp;

        m_driver->flush(m_framebuffers[m_frontBufIdx],
                        0, 0, m_driver->screenWidth, m_driver->screenHeight);

        xSemaphoreGive(m_bufferFree);
    }
    xSemaphoreGive(m_displayTaskExited);
    vTaskDelete(NULL);
}

void Renderer::initDualCore() {
    m_renderReady       = xSemaphoreCreateBinary();   // starts empty
    m_bufferFree        = xSemaphoreCreateBinary();   // starts empty
    m_displayTaskExited = xSemaphoreCreateBinary();   // starts empty

    xSemaphoreGive(m_bufferFree); // Prime: allow the very first render() call to proceed without deadlock.

    xTaskCreatePinnedToCore(Renderer::displayTask_wrapper,
        "WE_DispTask",
        DISPLAY_TASK_STACK_SIZE,
        this,
        DISPLAY_TASK_PRIORITY,
        &m_displayTaskHandle,
        DISPLAY_TASK_CORE_ID
    );
}

void Renderer::renderShutDown() {
    // Set flag before giving semaphore so the task sees it on wake-up.
    m_displayTaskShouldExit = true;
    xSemaphoreGive(m_renderReady);

    // Wait for the display task to confirm exit before deleting semaphores.
    xSemaphoreTake(m_displayTaskExited, portMAX_DELAY);

    vSemaphoreDelete(m_renderReady);
    vSemaphoreDelete(m_bufferFree);
    vSemaphoreDelete(m_displayTaskExited);
    m_renderReady       = nullptr;
    m_bufferFree        = nullptr;
    m_displayTaskExited = nullptr;
}

#endif
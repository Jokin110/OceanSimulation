#include <d3d11.h>
#include <cstdint>

class GpuProfiler {
private:
    static const int FrameDelay = 60; // N-frame lag to prevent CPU stalling

    ID3D11Query* m_disjointQueries[FrameDelay];
    ID3D11Query* m_startQueries[FrameDelay];
    ID3D11Query* m_endQueries[FrameDelay];

    int m_currentFrame;

public:
    GpuProfiler() : m_currentFrame(0) {
        // Zero memory to prevent garbage pointer crashes
        ZeroMemory(m_disjointQueries, sizeof(m_disjointQueries));
        ZeroMemory(m_startQueries, sizeof(m_startQueries));
        ZeroMemory(m_endQueries, sizeof(m_endQueries));
    }

    ~GpuProfiler() {
        for (int i = 0; i < FrameDelay; ++i) {
            if (m_disjointQueries[i]) m_disjointQueries[i]->Release();
            if (m_startQueries[i]) m_startQueries[i]->Release();
            if (m_endQueries[i]) m_endQueries[i]->Release();
        }
    }

    void Initialize(ID3D11Device* device) {
        D3D11_QUERY_DESC disjointDesc = { D3D11_QUERY_TIMESTAMP_DISJOINT, 0 };
        D3D11_QUERY_DESC timestampDesc = { D3D11_QUERY_TIMESTAMP, 0 };

        for (int i = 0; i < FrameDelay; ++i) {
            device->CreateQuery(&disjointDesc, &m_disjointQueries[i]);
            device->CreateQuery(&timestampDesc, &m_startQueries[i]);
            device->CreateQuery(&timestampDesc, &m_endQueries[i]);
        }
    }

    void Begin(ID3D11DeviceContext* context) {
        // Start the disjoint query and drop the start timestamp
        context->Begin(m_disjointQueries[m_currentFrame]);
        context->End(m_startQueries[m_currentFrame]);
    }

    void End(ID3D11DeviceContext* context) {
        // Drop the end timestamp and end the disjoint query
        context->End(m_endQueries[m_currentFrame]);
        context->End(m_disjointQueries[m_currentFrame]);
    }

    // Returns the elapsed time in milliseconds. Returns -1.0 if data isn't ready yet.
    double GetElapsedMsAndAdvanceFrame(ID3D11DeviceContext* context) {
        // We look at the oldest frame to avoid stalling the CPU
        int oldestFrame = (m_currentFrame + 1) % FrameDelay;

        D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
        uint64_t startTimestamp = 0;
        uint64_t endTimestamp = 0;
        double elapsedMs = -1.0;

        // Check if the data from N frames ago is ready
        if (context->GetData(m_disjointQueries[oldestFrame], &disjointData, sizeof(disjointData), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK) {
            if (context->GetData(m_startQueries[oldestFrame], &startTimestamp, sizeof(startTimestamp), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK &&
                context->GetData(m_endQueries[oldestFrame], &endTimestamp, sizeof(endTimestamp), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK) {

                // If Disjoint is true, the counter was interrupted (e.g., laptop changed power state).
                // In this case, the data is invalid, so we just ignore it this frame.
                if (!disjointData.Disjoint && disjointData.Frequency > 0) {
                    uint64_t deltaTicks = endTimestamp - startTimestamp;
                    elapsedMs = (static_cast<double>(deltaTicks) / disjointData.Frequency) * 1000.0;
                }
            }
        }

        // Advance to the next frame in the ring buffer
        m_currentFrame = (m_currentFrame + 1) % FrameDelay;

        return elapsedMs;
    }
};
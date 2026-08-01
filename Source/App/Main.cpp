// Minimal standalone entry for Ghost Signal MS20P.
// JUCE's 'Standalone' format target uses a generated main() when this file is provided.
#include <JuceHeader.h>
#include "PluginProcessor.h"

class GhostSignalApplication  : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override       { return "Ghost Signal MS20P"; }
    const juce::String getApplicationVersionString() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override             { return false; }

    void initialise (const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow> (getApplicationName());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    class MainWindow  : public juce::DocumentWindow
    {
    public:
        MainWindow (juce::String name)
            : juce::DocumentWindow (name,
                                    juce::Desktop::getInstance().getDefaultLookAndFeel()
                                        .findColour (juce::ResizableWindow::backgroundColourId),
                                    juce::DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setResizable (true, false);
            setSize (1200, 800);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (GhostSignalApplication)

import tkinter as tk
from mido import Message, open_output


class MidiControllerApp:
    def __init__(self, root, midi_port_name):
        self.root = root
        self.root.title("MIDI Controller")

        # Open the MIDI output port
        self.midi_out = open_output(midi_port_name)

        # Slider for Control Change messages
        self.slider_label = tk.Label(root, text="Slider (-8% to 8%)")
        self.slider_label.pack()
        self.slider = tk.Scale(
            root,
            from_=-8,
            to=8,
            orient=tk.HORIZONTAL,
            resolution=0.1,
            command=self.send_cc,
        )
        self.slider.pack()

        # Play button for Note On
        self.play_button = tk.Button(root, text="Play", command=self.send_play)
        self.play_button.pack()

        # Cue button for Note On
        self.cue_button = tk.Button(root, text="Cue", command=self.send_cue)
        self.cue_button.pack()

    def send_cc(self, value):
        # Map slider value to a MIDI CC value (scaled to 0-127)
        cc_value = int((float(value) + 8) * (127 / 16))
        print(f"Sending CC: {cc_value}")
        self.midi_out.send(Message("control_change", control=1, value=cc_value))

    def send_play(self):
        print("Sending Play (Note On: 60)")
        self.midi_out.send(Message("note_on", note=60, velocity=100))

    def send_cue(self):
        print("Sending Cue (Note On: 62)")
        self.midi_out.send(Message("note_on", note=62, velocity=100))

    def close(self):
        self.midi_out.close()


if __name__ == "__main__":
    # Create the main application window
    root = tk.Tk()

    # List available MIDI output ports
    import mido

    print("Available MIDI output ports:")
    for i, port in enumerate(mido.get_output_names()):
        print(f"[{i}] {port}")

    # Select a MIDI port
    port_index = int(input("Select the MIDI output port index: "))
    midi_port_name = mido.get_output_names()[port_index]

    # Create and run the app
    app = MidiControllerApp(root, midi_port_name)

    # Handle GUI window close event
    def on_closing():
        app.close()
        root.destroy()

    root.protocol("WM_DELETE_WINDOW", on_closing)
    root.mainloop()

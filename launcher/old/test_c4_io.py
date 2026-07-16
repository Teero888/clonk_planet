import unittest
import os
import tempfile
import struct
from c4_io import unscramble, scramble, C4Group, C4GroupWriter, parse_c4_text, update_c4_text

class TestC4IO(unittest.TestCase):
    def test_scramble_unscramble(self):
        original = b"This is a test header for Clonk 4 Group files."
        scrambled = scramble(original)
        unscrambled = unscramble(scrambled)
        self.assertEqual(original, unscrambled)
        self.assertNotEqual(original, scrambled)

    def test_c4_text_parsing(self):
        content = "[Player]\nName=Twonky\n[Preferences]\nColor=1\n[LastRound]\nScore=100"
        sections = parse_c4_text(content)
        self.assertEqual(sections['Player']['Name'], 'Twonky')
        self.assertEqual(sections['Preferences']['Color'], '1')
        self.assertEqual(sections['LastRound']['Score'], '100')

    def test_c4_text_update(self):
        content = "[Player]\nName=Twonky\n[Preferences]\nColor=1\n"
        new_values = {'Color': 5, 'Control': 0}
        updated = update_c4_text(content, 'Preferences', new_values)
        
        sections = parse_c4_text(updated)
        self.assertEqual(sections['Preferences']['Color'], '5')
        self.assertEqual(sections['Preferences']['Control'], '0')
        self.assertEqual(sections['Player']['Name'], 'Twonky')

    def test_group_writer_and_reader(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            group_path = os.path.join(tmpdir, "test.c4p")
            
            # 1. Write a group
            writer = C4GroupWriter()
            writer.add_file("Player.txt", b"[Player]\nName=TestClonk")
            writer.add_file("Icon.bmp", b"FAKEBMDATA")
            writer.write_to_file(group_path)
            
            self.assertTrue(os.path.exists(group_path))
            
            # 2. Read it back
            grp = C4Group(group_path)
            self.assertEqual(len(grp.entries), 2)
            
            player_txt = grp.get_file("Player.txt")
            self.assertEqual(player_txt, b"[Player]\nName=TestClonk")
            
            icon_bmp = grp.get_file("Icon.bmp")
            self.assertEqual(icon_bmp, b"FAKEBMDATA")

    def test_nested_group_reading(self):
        # Create a group that contains another group as a file
        with tempfile.TemporaryDirectory() as tmpdir:
            inner_path = os.path.join(tmpdir, "inner.c4p")
            outer_path = os.path.join(tmpdir, "outer.c4f")
            
            # Inner group
            i_writer = C4GroupWriter()
            i_writer.add_file("Data.txt", b"InnerData")
            i_writer.write_to_file(inner_path)
            
            with open(inner_path, 'rb') as f:
                inner_data = f.read()
                
            # Outer group
            o_writer = C4GroupWriter()
            o_writer.add_file("inner.c4p", inner_data)
            o_writer.write_to_file(outer_path)
            
            # Read back
            o_grp = C4Group(outer_path)
            inner_group_data = o_grp.get_file("inner.c4p")
            self.assertIsNotNone(inner_group_data)
            
            i_grp = C4Group(raw_data=inner_group_data)
            print(f"Inner entries: {[e['name'] for e in i_grp.entries]}")
            self.assertEqual(i_grp.get_file("Data.txt"), b"InnerData")

if __name__ == '__main__':
    unittest.main()

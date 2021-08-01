./cli bla.vox -o bla.glb
./cli *.vox -o *.glb -> test.vox -> test.glb
./cli *.vox -o Test_{0}.glb -> creates files with the names Test_0.glb Test_1.glb Test_N.glb
./cli FOLDER -o ABOVE
./cli *.* -o ABOVE
./cli file1.vox file2.vox file3.vox -o ABOVE
./cli FOLDER1 FOLDER2 -o ABOVE
./cli ABOVE -o OUTFILER/ABOVE


Get-ChildItem *.exe -Recurse | foreach($_) {Remove-Item $_}
Get-ChildItem *.obj -Recurse | foreach($_) {Remove-Item $_}	
Get-ChildItem *.pdb -Recurse | foreach($_) {Remove-Item $_}
Get-ChildItem *.exp -Recurse | foreach($_) {Remove-Item $_}
Get-ChildItem *.dll -Recurse | foreach($_) {Remove-Item $_}
Get-ChildItem *.lib -Recurse | foreach($_) {Remove-Item $_}
Get-ChildItem *.ilk -Recurse | foreach($_) {Remove-Item $_}
Get-ChildItem *.i -Recurse | foreach($_) {Remove-Item $_}

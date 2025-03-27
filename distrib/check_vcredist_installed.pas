//----------------------------------------------------------------------------
//  Проверка наличия уже установленного Visual C++ Redistributable
//----------------------------------------------------------------------------

function VC2022RedistNeedsInstall: Boolean;
var 
  Version: String;
begin
  if RegQueryStringValue(HKEY_LOCAL_MACHINE,
       'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Version',
       Version) then
  begin
    // Is the installed version at least 14.34 ? 
    Log('VC Redist Version check : found ' + Version);
    Result := (CompareStr(Version, 'v14.34') < 0);
  end
  else 
    Result := True;
end;

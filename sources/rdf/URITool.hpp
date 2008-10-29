#include <list>

struct URI
{
public:
  URI(const std::wstring& aInput)
  {
    size_t p(aInput.find(L'#'));

    // Take the fragment and query off the end if possible...
    std::wstring d;
    if (p != std::wstring::npos)
    {
      fragment = aInput.substr(p + 1);
      d = aInput.substr(0, p);
    }
    else
      d = aInput;

    p = aInput.find(L'?');
    if (p != std::wstring::npos)
    {
      query = d.substr(p + 1);
      d = d.substr(0, p);
    }

    // Take the scheme and authority off the front if possible...
    p = aInput.find(L':');
    if (p != std::wstring::npos)
    {
      scheme = d.substr(0, p);
      // Back out if the scheme would otherwise have a / in it...
      if (scheme.find(L'/') != std::wstring::npos)
        scheme = L"";
      else
        d = d.substr(p + 1);
    }

    // See if we have an authority...
    if (d[0] == L'/' && d[1] == L'/')
    {
      p = d.find(L'/', 2);
      if (p != std::wstring::npos)
      {
        authority = d.substr(2, p - 2);
        d = d.substr(p);
      }
      else
      {
        authority = d.substr(2);
        return;
      }
    }

    path = d;
  }

  URI(const URI& aCopy)
    : scheme(aCopy.scheme), authority(aCopy.authority), path(aCopy.path),
      query(aCopy.query), fragment(aCopy.fragment)
  {
    
  }

  URI(const URI& aBase, const URI& aRelative)
  {
    // RFC2396 5.2 rule 2...
    if (aRelative.scheme == L"" && aRelative.authority == L"" && aRelative.path == L"" &&
        aRelative.query == L"")
    {
      scheme = aBase.scheme;
      authority = aBase.authority;
      path = aBase.path;
      query = aBase.query;
      fragment = aRelative.fragment;
      return;
    }
    query = aRelative.query;
    fragment = aRelative.fragment;

    // rule 3. We don't use the optional workaround for invalid URIs in the second part...
    if (aRelative.scheme != L"")
    {
      scheme = aRelative.scheme;
      authority = aRelative.authority;
      path = aRelative.path;
      return;
    }
    else
      scheme = aBase.scheme;

    // rule 4...
    if (aRelative.authority == L"")
    {
      authority = aBase.authority;
      
      // rule 5...
      if (aRelative.path[0] != L'/')
      {
        // rule 6...
        std::list<std::wstring> parts;
        
        bool ensureTrailing = false;
        size_t p = 0, pp = 0;
        if (aBase.path == L"")
        {
          parts.push_back(L"");
        }
        else
        {
          while (true)
          {
            p = aBase.path.find(L'/', pp);
            if (p == std::wstring::npos)
              // Drop the last part...
              break;
            
            parts.push_back(aBase.path.substr(pp, p - pp));
            
            pp = p + 1;
          }
        }
        pp = 0;
        while (true)
        {
          p = aRelative.path.find(L'/', pp);
          if (p == std::wstring::npos)
          {
            parts.push_back(aRelative.path.substr(pp));
            break;
          }

          parts.push_back(aRelative.path.substr(pp, p - pp));

          pp = p + 1;
        }

        std::list<std::wstring>::iterator j;
        for (std::list<std::wstring>::iterator i(parts.begin()); i != parts.end(); i=j)
        {
          j = i;
          j++;

          if (*i == L".")
          {
            ensureTrailing |= (j == parts.end());
            parts.erase(i);
          }
          else if (*i == L"..")
          {
            ensureTrailing |= (j == parts.end());
            if (i == parts.begin())
              parts.erase(i);
            else
            {
              std::list<std::wstring>::iterator k(i);
              k--;
              parts.erase(i);
              parts.erase(k);
            }
          }
        }

        if (ensureTrailing)
        {
          j = parts.end();
          if (j == parts.begin() || *--j != L"")
            parts.push_back(L"");
        }

        bool first = true;
        for (std::list<std::wstring>::iterator i(parts.begin()); i != parts.end(); i++)
        {
          if (!first)
            path += L"/";
          else
            first = false;

          path += *i;
        }
        if (!first && path == L"")
          path += L"/";
      }
      else
        path = aRelative.path;
    }
    else
    {
      authority = aRelative.authority;
      path = aRelative.path;
    }
  }

  void
  operator=(const URI& aURI)
  {
    scheme = aURI.scheme;
    authority = aURI.authority;
    path = aURI.path;
    query = aURI.query;
    fragment = aURI.fragment;
  }

  std::wstring
  absoluteURI()
  {
    std::wstring res;
    bool showEmptyAuthority = false;
    if (scheme == L"file")
      showEmptyAuthority = true;

    if (scheme != L"")
    {
      res += scheme;
      res += L':';
    }

    if (showEmptyAuthority)
      res += L"//";

    if (authority != L"")
    {
      if (!showEmptyAuthority)
        res += L"//";
      res += authority;
    }

    res += path;

    if (query != L"")
    {
      res += L'?';
      res += query;
    }

    if (fragment != L"")
    {
      res += L'#';
      res += fragment;
    }

    return res;
  }

  std::wstring
  relativeURI(const URI& aRelativeTo)
  {
    std::wstring res;
    bool rollUp = false;

    if (scheme != L"" && aRelativeTo.scheme != scheme)
    {
      res += scheme;
      res += L':';
      rollUp = true;
    }

    bool needAuthority = (rollUp || aRelativeTo.authority != authority);
    if (needAuthority)
    {
      bool showEmptyAuthority = false;
      if (scheme == L"file")
        showEmptyAuthority = true;

      if (authority != L"" || showEmptyAuthority)
      {
        res += L"//";
        res += authority;
        rollUp = true;
      }
    }

    if (rollUp)
      res += path;
    else if (aRelativeTo.path != path)
    {
      size_t s = -1, l1 = path.length(), l2 = aRelativeTo.path.length(), l = (l1 < l2) ? l1 : l2;
      for (size_t i = 0; i < l; i++)
      {
        if (path[i] != aRelativeTo.path[i])
          break;
        if (path[i] == L'/')
          s = i;
      }
      // s now holds the index of the last common slash. For every slash after that, add ../
      l = s + 1;
      while (true)
      {
        l = aRelativeTo.path.find(L'/', l);
        if (l == std::wstring::npos)
          break;
        l++;
        res += L"../";
      }

      res += path.substr(s + 1);
      rollUp = true;
    }

    if (query != L"" && (rollUp || aRelativeTo.query != query))
    {
      res += L'?';
      res += query;
    }

    if (fragment != L"")
    {
      res += L'#';
      res += fragment;
    }

    return res;
  }

  std::wstring scheme, authority, path, query, fragment;
};

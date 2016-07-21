<!--- ----------------------------------------------------------------------------
  Title:  DataPull.cfc
 Author:  John Venable (JBV)
Purpose:  a gateway to grab webpage data
    Log:  08/04/2014 - JBV - Initial version.
---------------------------------------------------------------------------- --->
<cfcomponent displayname="datapull" hint="I am the datapull Gateway" output="false">

  <cfcontent type="application/json; charset=UTF-8">

  <cffunction access="remote" description="gets the ids of pages we do NOT want to migrate" name="getExcluded" returnType="string">
    <cfreturn FileRead("/cspot/www/www.nist.gov/com/donotmigrateids.txt")>
  </cffunction>

  <cffunction name="getAllCspotURLs" access="remote" output="no" hint="All Live Commonspot URLs" returnType="array" returnFormat="json">

    <cfquery name="qGetAllCspotURLs" dataSource="commonspot-nist">
      SELECT CASE WHEN p.uploaded = 1 THEN 'http://www.nist.gov' + s.UploadURL + u.PublicFileName
      WHEN PageType = 3 THEN 'http://www.nist.gov' + s.ImagesURL + p.FileName
      ELSE 'http://www.nist.gov' + s.SubSiteURL + p.FileName
      END AS PageURL, p.DateContentLastModified AS LastModified
      FROM SitePages p JOIN SubSites s ON p.SubSiteID = s.ID
      LEFT JOIN UploadedDocs u ON p.ID = u.PageID and VersionState = 2
      WHERE p.ApprovalStatus = 0
      ORDER BY p.DateContentLastModified DESC
    </cfquery>

    <cfreturn queryToArray(qGetAllCspotURLs)>

  </cffunction>

  <cffunction name="getIDList" access="remote" output="false" hint="I return a a custom element's data" returntype="array" returnFormat="json">
    <cfargument name="formid" displayname="FormID of the element to pull" required="yes" type="numeric" default="12836">
    <cfargument name="fromdate" displayname="From Date" required="yes" type="string" default="#dateformat(DateAdd("d", -1, now()), "yyyy-mm-dd")#">
    <cfargument name="todate" displayname="To Date" required="yes" type="string" default="#dateformat(now(), "yyyy-mm-dd")#">

    <cfset excludedIDs = this.getExcluded()>

    <cfquery name="qPullIdList" dataSource="commonspot-nist">
      SELECT DISTINCT dfv.PageID, sp.DateContentLastModified
      FROM Data_FieldValue dfv JOIN SitePages as sp ON dfv.PageID = sp.ID
      WHERE dfv.FormID = <cfqueryparam CFSQLType="CF_SQL_INTEGER" value="#arguments.formid#">  /* pass the Custom Element ID  */
        AND dfv.VersionState = 2
        AND sp.approvalstatus = 0
        AND dfv.PageID NOT IN (<cfqueryparam CFSQLType="CF_SQL_INTEGER" list="yes" separator="," value="#excludedIDs#">) /* do not migrate page IDs */
        AND sp.DateContentLastModified BETWEEN <cfqueryparam CFSQLType="CF_SQL_VARCHAR" value="#arguments.fromdate#"> AND <cfqueryparam CFSQLType="CF_SQL_VARCHAR" value="#arguments.todate#">
      ORDER BY sp.DateContentLastModified DESC, dfv.PageID
    </cfquery>

    <!--- I ran into an issue where having the last mod date in the query didn't work, so I sort the in
      the query above, and then do a q-of-q for the ids. it could potentially be removed if we find
      it doesn't matter that the date field doesn't interfere. --->
    <cfquery name="filteredQuery" dbtype="query">
      SELECT PageID FROM qPullIdList
    </cfquery>

    <cfreturn queryToArray(filteredQuery)>

  </cffunction>


  <cffunction name="getPageFromId" access="remote" output="false" hint="I return a a custom element's data" returntype="any" returnFormat="json">
    <cfargument name="pageid" displayname="PageID of the element to pull" required="yes" type="numeric" default="">
    <cfargument name="formid" displayname="FormID of the element to pull" required="yes" type="numeric" default="12836">

    <cfset excludedIDs = this.getExcluded()>

    <cfset getImages = "15531,6326,20242">
    <cftry>
    <!--- GET THE DATA --->
    <cfquery name="qGetPageFromId" dataSource="commonspot-nist">
      declare @formid int
      declare @t table (FieldName varchar(1000),FieldID int)
      declare @cmd nvarchar(max), @sql nvarchar(max)
      select @cmd = '', @sql = ''
      set @formid = <cfqueryparam CFSQLType="CF_SQL_INTEGER" value="#arguments.formid#">  /* pass the Custom Element ID for fieldnames */

      INSERT INTO @t
      SELECT DISTINCT fic.FieldName, map.FieldID
      FROM dbo.Data_FieldValue dfv
      JOIN FormInputControlMap map ON dfv.FormID = map.FormID AND dfv.FieldID = map.fieldID
      JOIN FormInputControl fic ON map.fieldID = fic.ID
      WHERE dfv.FormID IN (@formid,2444) AND VersionState = 2
      ORDER BY map.FieldID

      SELECT @sql = @sql + '['+ t.FieldName+'],' FROM @t t ORDER BY t.FieldID
      SET @sql = left(@sql, len(@sql)-1)

      SET @cmd = 'SELECT o.*
      FROM (SELECT sp.name as pagename, sp.OwnerID as cs_uid, u.UserID as owner_name, sp.Title as Page_Title, sp.DateAdded as created, sp.DateContentLastModified as changed,
      STUFF(( SELECT '','' + cast(metadataid AS NVARCHAR(MAX)) FROM eTaxonomy.dbo.articlemetadata t WHERE t.objectID = dfv.PageID FOR XML PATH('''')),1,1,'''') as taxonomy_ids,
      CAST(sp.Description as NVARCHAR(MAX)) AS Page_Description,
      CAST( CASE WHEN len(dfv.fieldValue) > 0 THEN dfv.fieldValue ELSE REPLACE(REPLACE(REPLACE(cast(dfv.MemoValue AS NVARCHAR(max)), CHAR(10), '' ''), CHAR(13), ''''), CHAR(9), '''') end as NVARCHAR(max)) as FieldValue, dfv.PageID, fic.FieldName, ss.SubSiteURL + sp.fileName AS path <cfif ListContains(getImages, arguments.formid)>, REPLACE(i.SrcURL,''CPIMAGE:'','''') AS ImageID, ''http://www.nist.gov'' + ss2.imagesURL + sp2.filename AS ImageURL, CAST(sp2.Description as NVARCHAR(MAX)) AS ImageAlt, ic.caption AS ImageCaption</cfif>
      FROM Data_FieldValue dfv
      JOIN FormInputControlMap map ON dfv.FormID = map.FormID and dfv.FieldID = map.fieldID
      JOIN FormInputControl fic ON map.fieldID = fic.ID
      JOIN SitePages as sp ON dfv.PageID = sp.ID
      JOIN SubSites as ss ON ss.ID = sp.SubSiteID
      JOIN eUser.dbo.Users u ON u.ID = sp.OwnerID
      <cfif ListContains(getImages, arguments.formid)>
        LEFT JOIN Data_image i ON dfv.PageID = i.PageID AND i.VersionState = 2
        LEFT JOIN Data_ImageWithCaption ic ON dfv.PageID = ic.PageID AND ic.VersionState = 2
        LEFT JOIN SitePages sp2 ON SUBSTRING (i.SrcURL,9,len(i.SrcURL)-8) = sp2.ID
        LEFT JOIN Subsites ss2 ON sp2.SubsiteID = ss2.ID
      </cfif>
      WHERE sp.approvalstatus = 0 AND dfv.VersionState = 2 AND dfv.PageID <> 0 AND dfv.PageID = ''#arguments.pageid#''
      ) dfv1
      PIVOT (max(dfv1.FieldValue) for dfv1.FieldName in ('+@sql+')) as o
      ORDER BY o.changed DESC'
      print @cmd
      exec (@cmd)
    </cfquery>

    <cfscript>
      // put the queried row data in a structure to serialize as a JSON Object
      pageData = StructNew();
      cols = listToArray(listsort(qGetPageFromId.columnlist, "textnocase"));
      for(row in qGetPageFromId) {
        for(col in cols) {
          StructInsert(pageData, lcase(col), CleanHighAscii(exportCleanup(row[col])));
          // if there are any tags (<) then let's parse the field for images.
        }
      }
      // put the images in their own field.
      StructInsert(pageData, "columnscleaned", cols);


      // If we have a "blank page type" that has columns, let us
      // be good citizens and pull whatever is inside those columns.
      if(arguments.formid EQ 2421) {
        // Set the page URL for later scraping
        pageurl = 'http://www.nist.gov' & qGetPageFromId.path;

        // Based on column amount, set the appropriate selectors
        // to retrieve the contents of the sidebar(s).
        sidebarselector = "";
        sidebar2selector = "";
        switch(#qGetPageFromId.fic_blank_page_column_amout#) {
          case "NSTIC 2 Column":
          case "Two Column Wide":
            sidebarselector = "##right-coll-wrapper-wide";
            break;
          case "Two Column":
          case "Two Column Wide Right":
            sidebarselector = "##right-coll-wrapper";
            break;
          case "Three Column":
            sidebarselector = "##left-coll-wrapper";
            sidebar2selector = "##right-coll-wrapper";
            break;
        }

        if (len(trim(sidebarselector))) {
          StructInsert(pageData, "sidebar", CleanHighAscii(exportCleanup(this.getPageContents(pageurl,sidebarselector))));
        } else {
          StructInsert(pageData, "sidebar", "");
        }

        if (len(trim(sidebar2selector))) {
          StructInsert(pageData, "sidebar2", CleanHighAscii(exportCleanup(this.getPageContents(pageurl,sidebar2selector))));
        } else {
          StructInsert(pageData, "sidebar2", "");
        }
      }
    </cfscript>

    <cfreturn serializejson(pageData)>

    <cfcatch>
      <cflog text="DataPull Error: getPageFromId: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn arrayNew(1)>
    </cfcatch>

    </cftry>

  </cffunction>

  <cffunction name="getCEList" access="remote" output="false" hint="I return a a custom element's data" returntype="array">
    <cfargument name="formid" displayname="FormID of the element to pull" required="yes" type="numeric" default="12836">
    <cfargument name="fromdate" displayname="From Date" required="yes" type="string" default="#dateformat(now(), "yyyy-mm-dd")#">

    <cfset variables.fromDate = dateformat(arguments.fromdate, "yyyy-mm-dd")>
    <!--- these are elements whose renderhandler(stupidly) have an image embedded
          currently, it's project-program, and instrument, these we'll pull with
          yet more joins, but at least it's now in a single recordset.
    --->
    <cfset excludedIDs = this.getExcluded()>

    <cfset getImages = "15531,6326,20242">
    <cftry>
    <cfquery name="pulldata" dataSource="commonspot-nist">
      declare @formid int
      declare @t table (FieldName varchar(1000),FieldID int)
      declare @cmd nvarchar(max), @sql nvarchar(max)
      select @cmd = '', @sql = ''
      set @formid = <cfqueryparam CFSQLType="CF_SQL_INTEGER" value="#arguments.formid#">  /* pass the Custom Element ID  */

      INSERT INTO @t
      SELECT DISTINCT fic.FieldName, map.FieldID
      FROM dbo.Data_FieldValue dfv
        JOIN FormInputControlMap map ON dfv.FormID = map.FormID AND dfv.FieldID = map.fieldID
        JOIN FormInputControl fic ON map.fieldID = fic.ID
      WHERE dfv.FormID = @formid AND VersionState = 2
      ORDER BY map.FieldID

      SELECT @sql = @sql + '['+ t.FieldName+'],' FROM @t t ORDER BY t.FieldID
      SET @sql = left(@sql, len(@sql)-1)

      SET @cmd = 'SELECT o.*
        FROM (SELECT sp.name as pagename, sp.OwnerID as cs_uid, u.UserID as owner_name, sp.Title as Page_Title, sp.DateAdded as created, sp.DateContentLastModified as changed,
        STUFF(( SELECT '','' + cast(metadataid AS NVARCHAR(MAX)) FROM eTaxonomy.dbo.articlemetadata t WHERE t.objectID = dfv.PageID FOR XML PATH('''')),1,1,'''') as taxonomy_ids,
        CAST(sp.Description as NVARCHAR(MAX)) AS Page_Description,
        CAST( CASE WHEN len(dfv.fieldValue)>0 THEN dfv.fieldValue ELSE REPLACE(REPLACE(REPLACE(cast(dfv.MemoValue AS NVARCHAR(max)), CHAR(10), '' ''), CHAR(13), ''''), CHAR(9), '''') END AS NVARCHAR(max)) AS FieldValue,
        CAST(case when len(dfv2.fieldValue) > 0 THEN dfv2.fieldValue ELSE REPLACE(REPLACE(REPLACE(cast(dfv2.MemoValue as NVARCHAR(max)), CHAR(10), '' ''), CHAR(13), ''''), CHAR(9), '''') END AS NVARCHAR(max)) as contact_block,
        dfv.PageID, fic.FieldName, ss.SubSiteURL + sp.fileName as path
      <cfif ListContains(getImages, arguments.formid)>, REPLACE(i.SrcURL,''CPIMAGE:'','''') AS ImageID, CAST(sp2.Description as NVARCHAR(MAX)) AS ImageAlt, ic.caption AS ImageCaption</cfif>
        FROM Data_FieldValue dfv
        LEFT JOIN Data_FieldValue dfv2 ON dfv.pageid = dfv2.pageid AND dfv2.FormID = 2444 AND  dfv2.VersionState = 2
        JOIN FormInputControlMap map ON dfv.FormID = map.FormID and dfv.FieldID = map.fieldID
        JOIN FormInputControl fic ON map.fieldID = fic.ID
        JOIN SitePages as sp ON dfv.PageID = sp.ID
        JOIN SubSites as ss ON ss.ID = sp.SubSiteID
        JOIN eUser.dbo.Users u ON u.ID = sp.OwnerID
      <cfif ListContains(getImages, arguments.formid)>
        LEFT JOIN Data_image i ON dfv.PageID = i.PageID AND i.VersionState = 2
        LEFT JOIN Data_ImageWithCaption ic ON dfv.PageID = ic.PageID AND ic.VersionState = 2
        LEFT JOIN SitePages sp2 ON SUBSTRING (i.SrcURL,9,len(i.SrcURL)-8) = sp2.ID
        LEFT JOIN Subsites ss2 ON sp2.SubsiteID = ss2.ID
      </cfif>
        WHERE  sp.approvalstatus = 0 AND CAST(sp.DateContentLastModified AS DateTime) > ''#arguments.fromDate#'' AND dfv.FormID = '+cast(@formid as varchar(20))+ ' and dfv.VersionState = 2 and dfv.PageID <> 0
        AND dfv.PageID NOT IN (#excludedIDs#)) dfv1
        PIVOT (max(dfv1.FieldValue) for dfv1.FieldName in ('+@sql+')) as o
        ORDER BY o.changed DESC'
       print @cmd
      exec (@cmd)
    </cfquery>
    <!--- --->
    <cfscript>
      var jsoup = createObject("java", "org.jsoup.Jsoup");
      var queryColumns = arraytolist(pulldata.getMeta().getColumnLabels());
      var cleanQuery = QueryNew("#queryColumns#,parsedimageIDs");
      var aImages = arrayNew(1);
    </cfscript>

    <cfoutput query="pulldata" group="PageID">
      <cfset QueryAddRow(cleanQuery)>
      <cfloop index="col" list="#queryColumns#">
        <cfif Find("FIC", col)>
          <cfset toparse = pulldata[col][currentRow]>
          <cfset doc = jsoup.parse(toparse)>
          <cfset elements = doc.select("img[id]")>
            <cfloop index="e" array="#elements#">
            <cfset matches = REMatchNoCase("[0-9]+", e.attr("id").toString())>
            <cfif IsArray(matches) AND arrayLen(matches)>
              <cfset ArrayAppend(aImages, matches[1])>
            </cfif>
          </cfloop>
        </cfif>
        <cfset QuerySetCell(cleanQuery, col, CleanHighAscii(exportCleanup(pulldata[col][currentRow])))>
      </cfloop>
      <cfset QuerySetCell(cleanQuery, "parsedimageIDs", arraytolist(aImages))>
      <cfset ArrayClear(aImages)>
    </cfoutput>

    <cfreturn queryToArray(cleanQuery)>

    <cfcatch>
      <cflog text="DataPull Error: getCEList: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn  arrayNew(1)>
    </cfcatch>

    </cftry>

  </cffunction>

  <cffunction name="getTaxonomyTerms" access="remote" output="false" hint="I return a NIST taxonomy terms" returntype="array">

    <cfquery name="qGetTaxonomyTerms" dataSource="commonspot-nist-custom">
      SELECT d1.MetaDataID AS termid, d1.MetaDataKeyword AS termname, d1.MetadataParentID AS parent_id, d2.MetaDataKeyword as parentname
      FROM MetaDataKeyword d1 LEFT JOIN MetaDataKeyword d2 ON d1.MetadataParentID = d2.MetaDataID
      WHERE d1.MetaDataBitmask IS NOT NULL
      ORDER BY d1.MetaDataBitmask
    </cfquery>

    <cfreturn queryToArray(qGetTaxonomyTerms)>

  </cffunction>

  <cffunction name="getUsers" access="remote" output="false" hint="I return a NIST commonspot users" returntype="array">
    <cftry>
    <cfquery name="qGetUsers" dataSource="commonspot-users">
      SELECT u.ID AS commonspotid, u.UserID, c.EmailAddress as email, u.LastLogin AS LastLogin,
      REPLACE(LEFT(LEFT((REPLACE(nullif(f2.FieldValue,''),'CPIMAGE:','')), CHARINDEX('|',REPLACE(nullif(f2.FieldValue,''),'CPIMAGE:',''))),(CHARINDEX('|',REPLACE(nullif(f2.FieldValue,''),'CPIMAGE:','')))) , '|', '') AS USERIMAGEID
      FROM Users u JOIN Contacts c ON u.ID = c.ContactID LEFT JOIN eContent.dbo.Data_FieldValue f1 ON c.EmailAddress = f1.FieldValue AND F1.FieldID = 6795 AND f1.VersionState = 2
      LEFT JOIN  eContent.dbo.Data_FieldValue f2 ON f1.PageID = f2.PageID AND f2.FieldID = 6801 AND f2.VersionState = 2
      WHERE u.IsDeleted = 0 AND c.EmailAddress IS NOT NULL
      ORDER BY LastLogin DESC
    </cfquery>

    <cfscript>
      var queryColumns = arraytolist(qGetUsers.getMeta().getColumnLabels());
      var cleanQuery = QueryNew("#queryColumns#,people_id");
    </cfscript>

    <cfoutput query="qGetUsers" group="commonspotid">
      <cfset QueryAddRow(cleanQuery)>
      <cfset QuerySetCell(cleanQuery, "people_id", getCPRID(lcase(qGetUsers.email)))>
      <cfset QuerySetCell(cleanQuery, "commonspotid", commonspotid)>
      <cfset QuerySetCell(cleanQuery, "UserID", UserID)>
      <cfset QuerySetCell(cleanQuery, "email", email)>
      <cfset QuerySetCell(cleanQuery, "LastLogin", LastLogin)>
      <cfset QuerySetCell(cleanQuery, "USERIMAGEID", USERIMAGEID)>
    </cfoutput>

    <cfreturn queryToArray(cleanQuery)>

    <cfcatch>
      <cflog text="DataPull Error: getUsers: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn  arrayNew(1)>
    </cfcatch>

    </cftry>

  </cffunction>

  <cffunction name="getUserProfiles" access="remote" output="false" hint="I return a NIST commonspot users" returntype="array">
    <cfargument name="fromdate" displayname="From Date" required="yes" type="string" default="#dateformat(now(), "yyyy-mm-dd")#">

    <cftry>
    <!--- returns: changed, FIC_staff_bio, FIC_staff_employment, FIC_staff_education, FIC_staff_selected_publications, FIC_staff_middle_initial --->
    <cfquery name="qGetUserProfiles" dataSource="commonspot-nist">
      declare @formid int
      declare @t table (FieldName varchar(1000),FieldID int)
      declare @cmd nvarchar(max), @sql nvarchar(max)
      select @cmd = '', @sql = ''
      set @formid = 6773

      INSERT INTO @t
      SELECT DISTINCT fic.FieldName, map.FieldID
      FROM dbo.Data_FieldValue dfv
      JOIN FormInputControlMap map ON dfv.FormID = map.FormID AND dfv.FieldID = map.fieldID
      JOIN FormInputControl fic ON map.fieldID = fic.ID
      WHERE dfv.FormID = @formid AND VersionState = 2 AND dfv.FieldID IN (6804, 6806, 6810, 6807, 6795, 15921)
      ORDER BY map.FieldID

      SELECT @sql = @sql + '['+ t.FieldName+'],' FROM @t t ORDER BY t.FieldID
      SET @sql = left(@sql, len(@sql)-1)

      SET @cmd = 'SELECT o.*
      FROM (SELECT sp.DateContentLastModified as changed,dfv.pageid,
      CAST( CASE WHEN len(dfv.fieldValue)>0 THEN dfv.fieldValue ELSE REPLACE(REPLACE(REPLACE(cast(dfv.MemoValue AS NVARCHAR(max)), CHAR(10), '' ''), CHAR(13), ''''), CHAR(9), '''') END AS NVARCHAR(max)) AS FieldValue,
      fic.FieldName
      FROM Data_FieldValue dfv
      LEFT JOIN Data_FieldValue dfv2 ON dfv.pageid = dfv2.pageid AND dfv2.FormID = 2444 AND  dfv2.VersionState = 2
      JOIN FormInputControlMap map ON dfv.FormID = map.FormID and dfv.FieldID = map.fieldID
      JOIN FormInputControl fic ON map.fieldID = fic.ID
      JOIN SitePages as sp ON dfv.PageID = sp.ID
      JOIN SubSites as ss ON ss.ID = sp.SubSiteID
      JOIN eUser.dbo.Users u ON u.ID = sp.OwnerID
      WHERE  sp.approvalstatus = 0 AND CAST(sp.DateContentLastModified AS DateTime) > ''#arguments.fromDate#'' AND dfv.FormID = '+cast(@formid as varchar(20))+ ' and dfv.VersionState = 2 and dfv.PageID <> 0
      ) dfv1
      PIVOT (max(dfv1.FieldValue) for dfv1.FieldName in ('+@sql+')) as o
      ORDER BY o.changed DESC'
      print @cmd
      exec (@cmd)
    </cfquery>

     <!---  --->
    <cfscript>
      var queryColumns = arraytolist(qGetUserProfiles.getMeta().getColumnLabels());
      var cleanQuery = QueryNew("#queryColumns#,people_id");
    </cfscript>

    <cfoutput query="qGetUserProfiles" group="PageID">
      <cfset QueryAddRow(cleanQuery)>
      <cfloop index="col" list="#queryColumns#">
        <cfif Find("FIC", col)>
          <cfset toparse = qGetUserProfiles[col][currentRow]>
        </cfif>
        <cfset QuerySetCell(cleanQuery, col, CleanHighAscii(exportCleanup(qGetUserProfiles[col][currentRow])))>
      </cfloop>
      <cfset QuerySetCell(cleanQuery, "people_id", getCPRID(lcase(FIC_staff_email)))>
    </cfoutput>

    <cfreturn queryToArray(cleanQuery)>



    <cfcatch>
      <cflog text="DataPull Error: getUserProfiles: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn  arrayNew(1)>
    </cfcatch>

    </cftry>

  </cffunction>



  <cffunction name="getImageIDs" access="remote" output="false" returnType="array" returnFormat="json">
    <cfargument name="fromdate" displayname="From Date" required="yes" type="string" default="#dateformat(now(), "yyyy-mm-dd")#">

    <cftry>
      <cfquery name="qGetImageIDs" dataSource="commonspot-nist">
        SELECT ID
        FROM SitePages
        WHERE Pagetype = 3
          AND ApprovalStatus = 0
          AND Dateadded > <cfqueryparam CFSQLType="CF_SQL_DATE" value="#arguments.fromDate#">
        ORDER BY Dateadded DESC
      </cfquery>

      <cfreturn queryToArray(qGetImageIDs)>

    <cfcatch>
      <cflog text="DataPull Error: getImageIDs: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn  arrayNew(1)>
    </cfcatch>

    </cftry>
  </cffunction>


  <cffunction name="getImageFromID" access="remote" output="false" returnType="any" returnFormat="json">
    <cfargument name="iid" displayname="Image ID" required="no" type="numeric">

    <cftry>

      <cfquery name="qGetImageFromID" dataSource="commonspot-nist">
        SELECT 'http://www.nist.gov' + s.IMAGESURL + p.FILENAME AS imageurl, p.OWNERID, p.DATEADDED as created, p.FILENAME,
        REPLACE(i.description, '"', '&quot;') AS imagedescription, p.ID, s.SUBSITEURL, d.fieldvalue as taxonomy_ids
        FROM SitePages p JOIN SubSites s ON p.SubSiteID = s.ID
        LEFT JOIN Data_FieldValue d ON p.ID = d.PageID AND d.fieldid=1577310
        LEFT JOIN ImageGallery i ON p.id = i.pageid AND i.VersionState = 2 JOIN ImageCategories ic ON i.categoryid = ic.ID
        WHERE 1 = 1
        <cfif structKeyExists(arguments, "iid")>
        AND p.id = <cfqueryparam CFSQLType="CF_SQL_INTEGER" value="#arguments.iid#">
        </cfif>
        ORDER BY created
      </cfquery>

      <cfscript>
        // put the queried row data in a structure to serialize as a JSON Object
        pageData = StructNew();
        cols = listToArray(listsort(qGetImageFromID.columnlist, "textnocase"));
        for(row in qGetImageFromID) {
          for(col in cols) {
            StructInsert(pageData, lcase(col), CleanHighAscii(exportCleanup(row[col])));
          }
        }
      </cfscript>

      <cfreturn serializejson(pageData)>

    <cfcatch>
      <cflog text="DataPull Error: getImageFromID: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn  arrayNew(1)>
    </cfcatch>

    </cftry>
  </cffunction>

  <cffunction name="getHeadshotImages" access="remote" output="no" returnType="array" returnFormat="json">
    <cftry>

      <cfquery name="qGetHeadshotImages" dataSource="commonspot-nist">
        SELECT p.id as pageid, p.DateContentLastModified AS changed,  'http://www.nist.gov' + s.ImagesURL + p.filename AS AuthorImageUrl
        FROM SitePages p JOIN SubSites s ON p.SubSiteID = s.ID
        WHERE p.ID IN (SELECT CASE
          WHEN CHARINDEX('|',FieldValue) > 0 THEN SUBSTRING(Fieldvalue, 9, (CHARINDEX('|',FieldValue)-9))
          ELSE SUBSTRING(Fieldvalue, 9, (LEN(FieldValue)-8)) END AS ids
          FROM Data_FieldValue d
          WHERE FieldID = 6801
                AND VersionState = 2
                AND FieldValue <> '')
        ORDER BY changed DESC
      </cfquery>

      <cfreturn queryToArray(qGetHeadshotImages)>

    <cfcatch>
      <cflog text="DataPull Error: qGetHeadshotImages: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn  arrayNew(1)>
    </cfcatch>

    </cftry>

  </cffunction>

  <cffunction name="getTimelineEvents" access="remote" output="no" returnType="array" returnFormat="json">
    <cftry>

      <cfquery name="qGetTimelineEvents" dataSource="commonspot-nist">
        declare @formid int
        declare @t table (FieldName varchar(1000),FieldID int)
        declare @cmd nvarchar(max), @sql nvarchar(max)
        select @cmd = '', @sql = ''
        set @formid = 2913286

        INSERT INTO @t
        SELECT DISTINCT fic.FieldName, map.FieldID
        FROM dbo.Data_FieldValue dfv
        JOIN FormInputControlMap map ON dfv.FormID = map.FormID AND dfv.FieldID = map.fieldID
        JOIN FormInputControl fic ON map.fieldID = fic.ID
        WHERE dfv.FormID IN (@formid) AND VersionState = 2
        ORDER BY map.FieldID

        SELECT @sql = @sql + '['+ t.FieldName+'],' FROM @t t ORDER BY t.FieldID
        SET @sql = left(@sql, len(@sql)-1)

        SET @cmd = 'SELECT o.*
        FROM (SELECT
        CAST(CASE WHEN len(dfv.fieldValue)>0 THEN dfv.fieldValue
        ELSE REPLACE(REPLACE(REPLACE(CAST(dfv.MemoValue AS NVARCHAR(max)), CHAR(10), '' ''), CHAR(13), ''''), CHAR(9), '''') END AS NVARCHAR(MAX)) AS FieldValue,
        dfv.PageID, fic.FieldName
        FROM Data_FieldValue dfv
        JOIN FormInputControlMap map ON dfv.FormID = map.FormID and dfv.FieldID = map.fieldID
        JOIN FormInputControl fic ON map.fieldID = fic.ID
        WHERE dfv.FormID = '+cast(@formid as varchar(20))+ ' and dfv.VersionState = 2 and dfv.PageID <> 0) dfv1
        PIVOT (max(dfv1.FieldValue) for dfv1.FieldName in ('+@sql+')) as o
        ORDER BY PageID'
        print @cmd
        exec (@cmd)
      </cfquery>

      <cfset temp = QueryAddColumn(qGetTimelineEvents, "fic_event_image_alt", "VarChar", ArrayNew(1))>

      <cfoutput query="qGetTimelineEvents">
        <cfset QuerySetCell(qGetTimelineEvents, "fic_event_image_alt", ListLast(fic_event_image,"|"), qGetTimelineEvents.currentRow)>
        <cfset QuerySetCell(qGetTimelineEvents, "fic_event_image", ListLast(ListFirst(fic_event_image,"|"), ":"), qGetTimelineEvents.currentRow)>
      </cfoutput>

      <cfreturn queryToArray(qGetTimelineEvents)>

    <cfcatch>
      <cflog text="DataPull Error: qGetTimelineEvents: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn  arrayNew(1)>
    </cfcatch>

    </cftry>

  </cffunction>


  <cffunction name="getFileIDs" access="remote" output="false" returnType="array" returnFormat="json">
    <cfargument name="fromdate" displayname="From Date" required="yes" type="string" default="#dateformat(now(), "yyyy-mm-dd")#">
    <cftry>

      <cfquery name="qGetFileIDs" dataSource="commonspot-nist">
        SELECT ID
        FROM SitePages
        WHERE Pagetype = 0 and Uploaded = 1
          AND ApprovalStatus = 0
          AND Dateadded > <cfqueryparam CFSQLType="CF_SQL_DATE" value="#arguments.fromDate#">
        ORDER BY Dateadded DESC
      </cfquery>

      <cfreturn queryToArray(qGetFileIDs)>

    <cfcatch>
      <cflog text="DataPull Error: getFileIDs: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn  arrayNew(1)>
    </cfcatch>

    </cftry>
  </cffunction>

  <cffunction name="getFileFromID" access="remote" output="false" returnType="any" returnFormat="json">
    <cfargument name="fid" displayname="File ID" required="no" type="numeric">

    <cftry>

      <cfquery name="qGetFileFromID" dataSource="commonspot-nist">
        SELECT p.id AS pageid, p.name, p.title, p.Description, p.DateAdded as created, p.DateContentLastMajorRevision as changed, p.OwnerID AS uid, p.doctype,
        p.filename, 'http://www.nist.gov/_cs_upload' + s.SubSiteURL + p.filename as url, s.SubSiteURL AS dir, u.PublicFileName as OriginalFileName
        FROM SitePages p JOIN SubSites s ON p.SubSiteID = s.ID JOIN UploadedDocs u ON p.ID = u.PageID AND u.VersionState = 2
        WHERE 1=1
        <cfif structKeyExists(arguments, "fid")>
        AND p.id = <cfqueryparam CFSQLType="CF_SQL_INTEGER" value="#arguments.fid#">
        </cfif>
        ORDER BY changed DESC
      </cfquery>

      <cfscript>
        queryColumns = arraytolist(qGetFileFromID.getMeta().getColumnLabels());
        cleanQuery = QueryNew("#queryColumns#");
      </cfscript>

      <cfscript>
        // put the queried row data in a structure to serialize as a JSON Object
        pageData = StructNew();
        cols = listToArray(listsort(qGetFileFromID.columnlist, "textnocase"));
        for(row in qGetFileFromID) {
          for(col in cols) {
            StructInsert(pageData, lcase(col), CleanHighAscii(exportCleanup(row[col])));
          }
        }
      </cfscript>

      <cfreturn serializejson(pageData)>

    <cfcatch>
      <cflog text="DataPull Error: getFiles: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn  arrayNew(1)>
    </cfcatch>
    </cftry>

  </cffunction>

  <cffunction name="getNonCePages" access="remote" output="no" returnType="array" returnFormat="json">

    <cfargument name="to" displayname="To Date" required="no" type="string" default="#dateformat(now(), 'yyyy-mm-dd')#">
    <cfargument name="from" displayname="From Date" required="no" type="string" default="#dateformat(DateAdd('m', -6, now()), 'yyyy-mm-dd')#">

    <cfset excludedIDs = this.getExcluded()>

      <cfquery name="qGetNonCePages" datasource="commonspot-nist">
        SELECT sp.id as pageId, sp.name as pagename, sp.OwnerID as cs_uid, sp.DateContentLastModified AS changed, sp.Title as Page_Title,
        sp.DateAdded as created, df.FieldValue as pagetype, ss.SubSiteURL + sp.fileName as path, 'http://www.nist.gov' + ss.SubSiteURL + sp.fileName as pageurl,
        CAST(sp.Description AS NVARCHAR(MAX)) AS Page_Description, '' AS body, '' AS sidebar, '' AS sidebar2, u.UserID as owner_name,
        STUFF(( SELECT ',' + cast(metadataid AS NVARCHAR(MAX)) FROM eTaxonomy.dbo.articlemetadata t WHERE t.objectID = df.PageID FOR XML PATH('')),1,1,'') as taxonomy_ids
        FROM dbo.Data_FieldValue df
        JOIN sitepages sp on df.pageID=sp.id
        JOIN subsites ss on sp.subsiteid=ss.id
        JOIN eUser.dbo.Users u ON u.ID = sp.OwnerID
        WHERE FieldValue IN ('2-Column-Wide-Right','2-Column-Wide','3-Column,1-Column','2-Column-Wide-Right-No-Line','2-Column-Wide-Color-Column','2-Column','2-Column-50-50')
        AND fieldid=2514
        AND df.versionstate = 2
        AND sp.approvalstatus = 0
        AND PageType = 0
        AND PageId NOT IN (SELECT DISTINCT PageID FROM Data_FieldValue WHERE FormID = 1544965)
        AND PageID NOT IN (<cfqueryparam CFSQLType="CF_SQL_INTEGER" list="yes" separator="," value="#excludedIDs#">)
        AND sp.DateContentLastModified BETWEEN <cfqueryparam CFSQLType="CF_SQL_DATE" value="#arguments.from#">
          AND <cfqueryparam CFSQLType="CF_SQL_DATE" value="#arguments.to#">
        ORDER BY changed DESC
      </cfquery>

      <cfoutput query="qGetNonCePages">
        <cfset bodyselector = "##CS_Element_leftColContainer">
        <cfset sidebarselector = "">
        <cfset sidebar2selector = "">
        <cfswitch expression="#qGetNonCePages.pagetype#">
          <cfcase value="1-Column">
            <cfset bodyselector = "##CS_Element_leftColContainer">
          </cfcase>
          <cfcase value="2-Column,2-Column-Wide,2-Column-Wide-Color-Column,2-Column-Wide-Right,2-Column-Wide-Right-No-Line">
            <cfset bodyselector = "##CS_Element_leftColContainer">
            <cfset sidebarselector = "##CS_Element_centerColContainer">
          </cfcase>
          <cfcase value="2-Column-50-50">
            <cfset bodyselector = "##CS_Element_leftColContainer,##CS_Element_centerColContainer">
          </cfcase>
          <cfcase value="3-Column">
            <cfset bodyselector = "##CS_Element_centerColContainer">
            <cfset sidebarselector = "##CS_Element_leftColContainer">
            <cfset sidebar2selector = "##CS_Element_rightColContainer">
          </cfcase>
        </cfswitch>

        <!--- <cfset QuerySetCell(qGetNonCePages, "body", this.getPageContents(qGetNonCePages.pageurl,bodyselector))> --->
        <cfset QuerySetCell(qGetNonCePages, "body", "this is the body tag")>

        <cfif len(trim(sidebarselector))>
          <cfset QuerySetCell(qGetNonCePages, "sidebar", CleanHighAscii(exportCleanup(this.getPageContents(qGetNonCePages.pageurl,sidebarselector))))>
        </cfif>

        <cfif len(trim(sidebar2selector))>
          <cfset QuerySetCell(qGetNonCePages, "sidebar2", CleanHighAscii(exportCleanup(this.getPageContents(qGetNonCePages.pageurl,sidebar2selector))))>
        </cfif>
      </cfoutput>

    <cfreturn queryToArray(qGetNonCePages)>

  </cffunction>

  <cffunction name="getNonCePageIds" access="remote" output="false" returnType="array" returnFormat="json">
    <cfargument name="fromdate" displayname="From Date" required="yes" type="string" default="#dateformat(now(), "yyyy-mm-dd")#">

    <cfset excludedIDs = this.getExcluded()>

    <cftry>

      <cfquery name="qGetNonCePages" dataSource="commonspot-nist">
        SELECT sp.ID
        FROM dbo.Data_FieldValue df JOIN sitepages sp on df.pageID=sp.id JOIN subsites ss on sp.subsiteid=ss.id
        WHERE FieldValue IN ('2-Column-Wide-Right','2-Column-Wide','3-Column,1-Column','2-Column-Wide-Right-No-Line','2-Column-Wide-Color-Column','2-Column','2-Column-50-50')
        AND fieldid=2514
        AND df.versionstate = 2
        AND sp.approvalstatus = 0
        AND PageType = 0
        AND PageId NOT IN (SELECT DISTINCT PageID FROM Data_FieldValue WHERE FormID = 1544965)
        AND PageId NOT IN (<cfqueryparam CFSQLType="CF_SQL_INTEGER" list="yes" separator="," value="#excludedIDs#">)
        AND sp.DateContentLastModified > <cfqueryparam CFSQLType="CF_SQL_VARCHAR" value="#arguments.fromdate#">
        ORDER BY sp.DateContentLastModified DESC
      </cfquery>

      <cfreturn queryToArray(qGetNonCePages)>

    <cfcatch>
      <cflog text="DataPull Error: getNonCePageIds: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn  arrayNew(1)>
    </cfcatch>
    </cftry>

  </cffunction>

  <cffunction name="getNonCePageById" access="remote" output="false" returnType="any" returnFormat="json">

    <cfargument name="pageid" displayname="PageID of the element to pull" required="yes" type="numeric" default="">

    <cfset excludedIDs = this.getExcluded()>
    <cfset var errorReturn = structNew() />

    <cftry>

      <cfquery name="qGetNonCePageById" dataSource="commonspot-nist">
        SELECT sp.id, sp.name as pagename, sp.OwnerID as cs_uid, u.UserID as owner_name, sp.Title as Page_Title,
        sp.DateAdded as created, sp.DateContentLastModified as changed, dfv.FieldValue as pagetype, ss.SubSiteURL + sp.fileName as path,
         'http://www.nist.gov' + ss.SubSiteURL + sp.fileName as pageurl,
        STUFF(( SELECT ',' + CAST(metadataid AS NVARCHAR(MAX)) FROM eTaxonomy.dbo.articlemetadata t WHERE t.objectID = dfv.PageID FOR XML PATH('')),1,1,'') as taxonomy_ids,
        CAST(sp.Description as NVARCHAR(MAX)) AS Page_Description
        FROM Sitepages sp JOIN Data_FieldValue dfv ON sp.ID = dfv.PageID AND dfv.fieldid=2514
        JOIN FormInputControlMap map ON dfv.FormID = map.FormID and dfv.FieldID = map.fieldID
        JOIN FormInputControl fic ON map.fieldID = fic.ID
        JOIN SubSites as ss ON ss.ID = sp.SubSiteID
        JOIN eUser.dbo.Users u ON u.ID = sp.OwnerID
        WHERE  sp.approvalstatus = 0
        AND PageId = <cfqueryparam CFSQLType="CF_SQL_INTEGER" value="#arguments.pageid#">
        AND PageId NOT IN (<cfqueryparam CFSQLType="CF_SQL_INTEGER" list="yes" separator="," value="#excludedIDs#">)
        ORDER BY changed DESC
      </cfquery>

      <cfscript>
        //create structure to hold query data
        pageData = StructNew();
        cols = listToArray(listsort(qGetNonCePageById.columnlist, "textnocase"));
        for(row in qGetNonCePageById) {
          for(col in cols) {
            StructInsert(pageData, lcase(col), row[col]);
          }
        }

        // Based on content type renderhandler, set the appropriate selectors
        sidebarselector = "";
        sidebar2selector = "";
        switch(#qGetNonCePageById.pagetype#) {
          case "1-Column":
            bodyselector = "##CS_Element_leftColContainer";
            break;
          case "2-Column":
          case "2-Column-Wide":
          case "2-Column-Wide-Color-Column":
          case "2-Column-Wide-Right":
          case "2-Column-Wide-Right-No-Line":
            bodyselector = "##CS_Element_leftColContainer";
            sidebarselector = "##CS_Element_centerColContainer";
            break;
          case "2-Column-50-50":
            bodyselector = "##CS_Element_leftColContainer";
            break;
          case "3-Column":
            bodyselector = "##CS_Element_centerColContainer";
            sidebarselector = "##CS_Element_leftColContainer";
            sidebar2selector = "##CS_Element_rightColContainer";
            break;
        }

        StructInsert(pageData, "body", CleanHighAscii(exportCleanup(this.getPageContents(qGetNonCePageById.pageurl,bodyselector))));

        StructInsert(pageData, "parsedimageIDs", parseImageIds(this.getPageContents(qGetNonCePageById.pageurl,bodyselector)));

        if (len(trim(sidebarselector))) {
          StructInsert(pageData, "sidebar", CleanHighAscii(exportCleanup(this.getPageContents(qGetNonCePageById.pageurl,sidebarselector))));
        } else {
          StructInsert(pageData, "sidebar", "");
        }

        if (len(trim(sidebar2selector))) {
          StructInsert(pageData, "sidebar2", CleanHighAscii(exportCleanup(this.getPageContents(qGetNonCePageById.pageurl,sidebar2selector))));
        } else {
          StructInsert(pageData, "sidebar2", "");
        }
      </cfscript>

      <cfreturn pageData>

    <cfcatch>
      <cflog text="DataPull Error: qGetNonCePageById: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">

      <cfset errorReturn.message = cfcatch.Message />

      <cfreturn errorReturn >
    </cfcatch>
    </cftry>

  </cffunction>

  <cffunction name="testJson" access="remote" output="false" returnType="any" returnFormat="json">

    <cfscript>
      jsontestdata = StructNew();
      StructInsert(jsontestdata, "taxonomy_ids", "130,550,554");
      StructInsert(jsontestdata, "pagename", "sub-wildlife");
      StructInsert(jsontestdata, "owner_name", "stanleyc");
      StructInsert(jsontestdata, "changed", "2015-05-26 15:12:14");
      StructInsert(jsontestdata, "page_description", "OSAC Wildlife Forensics Subcommittee");
      StructInsert(jsontestdata, "cs_uid", "1000036");
      StructInsert(jsontestdata, "page_title", "OSAC Wildlife Forensics Subcommittee");
      StructInsert(jsontestdata, "path", "/forensics/osac/sub-wildlife.cfm");
      StructInsert(jsontestdata, "pageurl", "http=//www.nist.gov/forensics/osac/sub-wildlife.cfm");
      StructInsert(jsontestdata, "created", "2014-07-11 11:44:08");
      </cfscript>

      <cfreturn serializeJSON(jsontestdata)>

  </cffunction>

  <!--- getCachedFileContent --->
  <cffunction name="getCachedFileContent" output="true" returntype="any"
    displayname="Get Cached File Content" hint="I return cached file content."
    description="I return the contents of a file from a cache.">

    <cfargument name="path" required="true" type="string"
      hint="I am the full system path to the file. I am required." />
    <cfargument name="scopeKey" required="false" type="string" default="someFileExclusionJSONList"
      hint="I am the key to store the data in. I default to 'someFileExclusionJSONList'." />
    <cfargument name="minutesToCache" required="false" type="numeric" default="60"
      hint="I am the minutes to keep data around before I re-read a file and replace it. I default to 60." />
    <cfargument name="killServerContainer" required="false" type="boolean" default="false"
      hint="I am a flag that if true will delete the container struct we store stuff in." />

    <cfset var theFileData = '' />

    <!---
      Should we kill the main container? this is really just to be cool so
      if calling code want's to be nice and clean up after itself at some
      point in the future it can.
    --->
    <cfif killServerContainer>
      <cfset structDelete( server, 'nistMigrationContainer' ) />
      <cfreturn />
    </cfif>

    <!--- create the containing struct we'll hold the cached lists in --->
    <cfif not structKeyExists( server, 'nistMigrationContainer' )>
      <cfset server.nistMigrationContainer = structNew() />
    </cfif>

    <!--- create the struct if it's not allready there and make the date 63 min old --->
    <cfif not structKeyExists( server.nistMigrationContainer, arguments.scopeKey )>
      <cfset server.nistMigrationContainer[arguments.scopeKey] = structNew() />
      <cfset server.nistMigrationContainer[arguments.scopeKey].created = dateAdd( 'n', -63, now() ) />
      <cfset server.nistMigrationContainer[arguments.scopeKey].fileData = '' />
    </cfif>

    <!--- if the data is older than 60 min reset created and read the file --->
    <cfif dateDiff( 'n', server.nistMigrationContainer[arguments.scopeKey].created, now() ) gt minutesToCache>
      <cfset server.nistMigrationContainer[arguments.scopeKey].created = now() />
      <cffile action="read" file="#arguments.path#" variable="theFileData" />
      <cfset server.nistMigrationContainer[arguments.scopeKey].fileData = theFileData />
    </cfif>

    <cfreturn server.nistMigrationContainer[arguments.scopeKey].fileData />
  </cffunction>


  <cffunction name="getPageContents" access="remote" output="false" returnType="string">

    <cfargument name="url" displayname="URL of the page to grab" required="yes" type="string" default="">
    <cfargument name="selector" displayname="What part to grab" required="yes" type="string" default="##CS_Element_leftColContainer">

    <cfset outputString = "">

    <cftry>

      <cfif isValid("url",arguments.url)>
        <!--- cache it to speed it up --->
        <cfif not cacheIdExists(arguments.url)>
          <cfhttp url="#arguments.url#" timeout="45">
          <cfset html = cfhttp.filecontent>
          <cfset cachePut(arguments.url,html)>
        <cfelse>
          <cfset html = cacheGet(arguments.url)>
        </cfif>

        <cfset jsoup = createObject("java", "org.jsoup.Jsoup")>
        <cfset doc = jsoup.parse(html)>
        <cfset elements = doc.select(arguments.selector)>
        <cfset translate = elements.select(".noexport").remove()>

        <cfloop index="e" array="#elements#">
          <cfset outputString = outputString & e.toString()>
        </cfloop>

      </cfif>

      <cfreturn outputString>

    <cfcatch>
      <cflog text="DataPull Error: getPageContents: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn  arrayNew(1)>
    </cfcatch>
    </cftry>

  </cffunction>

  <cffunction name="getCPRID" access="remote" output="false" returnType="string">
    <cfargument name="csemail" displayname="email from CS" required="yes" type="string" default="">

    <cfquery name="qGetCPRID" dataSource="cprproxy">
      SELECT DISTINCT people_id
      FROM emails
      WHERE email_address = <cfqueryparam CFSQLType="CF_SQL_VARCHAR" value="#arguments.csemail#">
    </cfquery>

    <cfreturn qGetCPRID.people_id>

  </cffunction>

<cffunction name="CleanHighAscii" access="private" returntype="string" output="false" hint="Cleans extended ascii values to make the as web safe as possible.">
  <!--- Define arguments. --->
  <cfargument name="Text" type="string" required="true" hint="The string that we are going to be cleaning." />
  <!--- Set up local scope. --->
  <cfset var LOCAL = {} />
  <!---
    When cleaning the string, there are going to be ascii values that we want to target, but there are also going
    to be high ascii values that we don't expect. Therefore, we have to create a pattern that simply matches all non
    low-ASCII characters. This will find all characters that are NOT in the first 127 ascii values. To do this, we
    are using the 2-digit hex encoding of values.
  --->
  <cfset LOCAL.Pattern = CreateObject("java", "java.util.regex.Pattern").Compile(JavaCast( "string", "[^\x20-\x7E\x0D\x09]" )) />
  <!---
    Create the pattern matcher for our target text. The matcher will be able to loop through all the high
    ascii values found in the target string.
  --->
  <cfset LOCAL.Matcher = LOCAL.Pattern.Matcher(JavaCast( "string", ARGUMENTS.Text )) />
  <!---
    As we clean the string, we are going to need to build a results string buffer into which the Matcher will
    be able to store the clean values.
  --->
  <cfset LOCAL.Buffer = CreateObject("java","java.lang.StringBuffer").Init() />
  <!--- Keep looping over high ascii values. --->
  <cfloop condition="LOCAL.Matcher.Find()">
    <!--- Get the matched high ascii value. --->
    <cfset LOCAL.Value = LOCAL.Matcher.Group() />
    <!--- Get the ascii value of our character. --->
    <cfset LOCAL.AsciiValue = Asc( LOCAL.Value ) />
    <!---
      Now that we have the high ascii value, we need to figure out what to do with it. There are explicit
      tests we can perform for our replacements. However, if we don't have a match, we need a default
      strategy and that will be to just store it as an escaped value.
    --->
    <!--- Check for Microsoft double smart quotes. --->
    <cfif ((LOCAL.AsciiValue EQ 8220) OR (LOCAL.AsciiValue EQ 8221))>
      <!--- Use standard quote. --->
      <cfset LOCAL.Value = """" />
    <!--- Check for Microsoft single smart quotes. --->
    <cfelseif ((LOCAL.AsciiValue EQ 8216) OR (LOCAL.AsciiValue EQ 8217))>
      <!--- Use standard quote. --->
      <cfset LOCAL.Value = "'" />
    <!--- Check for Microsoft elipse. --->
    <cfelseif (LOCAL.AsciiValue EQ 8230)>
      <!--- Use several periods. --->
      <cfset LOCAL.Value = "..." />
    <cfelseif (LOCAL.AsciiValue EQ 961)>
      <cfset LOCAL.Value = "&rho;" />
    <cfelse>
      <!---
        We didn't get any explicit matches on our character, so just store the escaped value.
      --->
      <cfset LOCAL.Value = "&###LOCAL.AsciiValue#;" />
    </cfif>
    <!---
      Add the cleaned high ascii character into the results buffer. Since we know we will only be working with extended values,
      we know that we don't have to worry about escaping any special characters in our target string.
    --->
    <cfset LOCAL.Matcher.AppendReplacement(LOCAL.Buffer,JavaCast( "string", LOCAL.Value )) />
  </cfloop>
  <!---
    At this point there are no further high ascii values in the string. Add the rest of the target text to the results buffer.
  --->
  <cfset LOCAL.Matcher.AppendTail(LOCAL.Buffer) />
  <!--- Return the resultant string. --->
  <cfreturn LOCAL.Buffer.ToString() />
</cffunction>


<!--- prvate parts --->
<cfscript>
  function parseImageIds(input) {
    jsoup = createObject("java", "org.jsoup.Jsoup");
    doc = jsoup.parse(input);
    imageIds = "";
    elements = doc.select("img[id]");
    for(i=1; i <= ArrayLen(elements); i++) {
      matches = REMatchNoCase("[0-9]+", elements[i].attr("id").toString());
      imageIds = listappend(imageIds, matches[1]);
    }
    return imageIds;
  }


  private function queryToArray(q) {
    var s = [];
    var cols = q.columnList;
    var colsLen = listLen(cols);
    for(var i=1; i<=q.recordCount; i++) {
      var row = {};
      for(var k=1; k<=colsLen; k++) {
        row[lcase(listGetAt(cols, k))] = q[listGetAt(cols, k)][i];
      }
      arrayAppend(s, row);
    }
    return s;
  }

  private function exportCleanup(texttoclean){
    cleantext = REReplaceNoCase(texttoclean,"#chr(13)#|#chr(9)#|\n|\r|\t","","ALL");
    // unescape quotes
    cleantext = replace(cleantext, "&quot;","""", "ALL");
    // get rid of stupid mouse events that CS puts in
    cleantext = replace(cleantext, " onmouseover=""""","", "ALL");
    cleantext = replace(cleantext, " onmouseout=""""","", "ALL");
    // get rid of empty style and summary tags
    cleantext = replace(cleantext, " style=""""","", "ALL");
    cleantext = replace(cleantext, " summary=""""","", "ALL");
      // unescape stuff that CS escapes needlessly
    cleantext = replace(cleantext, "&amp;","&", "ALL");
    cleantext = replace(cleantext, "&nbsp;"," ", "ALL");
    cleantext = replace(cleantext, "&##39;","'", "ALL");
    // empty paragraph tags must go
    cleantext = replace(cleantext, "<p></p>","", "ALL");
    cleantext = replace(cleantext, "<p>&##160;</p>","", "ALL");
    cleantext = replace(cleantext, "<p>&nbsp;</p>","", "ALL");
    cleantext = replace(cleantext, "<p> </p>","", "ALL");

    // bad CS ids
    // cleantext = REReplaceNoCase(cleantext, " id=""([^""]+)""", "", "ALL");

    // killing... we need anchor tags.
    //cleantext = REReplaceNoCase(cleantext, "<a name=""([^""]+)""></a>", " ", "ALL");

    // get rid of more stupid mouse events that CS puts in
    cleantext = REReplaceNoCase(cleantext, " onmouseover=""([^""]+)""", "", "ALL");
    cleantext = REReplaceNoCase(cleantext, " onmouseout=""([^""]+)""", "", "ALL");
    // popups that depend on CS are useless
    cleantext = REReplaceNoCase(cleantext,"javascript:HandleLink\('cpe_0_0','CPNEWWIN:NewWindow%5Etop=10,left=10,width=500,height=400,toolbar=1,location=1,directories=0,status=1,menubar=1,scrollbars=1,resizable=1@CP___PAGEID=([0-9]+),","","ALL");
    cleantext = REReplaceNoCase(cleantext,"javascript:HandleLink\('cpe_0_0','CPNEWWIN:NewWindow\^top=10,left=10,width=500,height=400,toolbar=1,location=1,directories=0,status=1,menubar=1,scrollbars=1,resizable=1@","","ALL");
    cleantext = REReplaceNoCase(cleantext, "'\);","","ALL");
    // get rid of links with the exit script, both new and old
    cleantext = replace(cleantext, "http://www.nist.gov/cgi-bin/exit_nist.cgi?url=","", "ALL");
    cleantext = replace(cleantext, "http://www.nist.gov/nist-exit-script.cfm?url=","", "ALL");
    cleantext = replace(cleantext, "/cgi-bin/exit_nist.cgi?url=","", "ALL");
    cleantext = replace(cleantext, "/nist-exit-script.cfm?url=","", "ALL");
    // styles begone
    cleantext = rereplace(cleantext," (\bstyle\s*=\s*('[^']*'|""[^""]*""|[^\s>]+))","","ALL");
    // kill microsoft xml
    cleantext = rereplace(cleantext, "<!--\[if.*?<!\[endif\]-->","","ALL");
    //kill MS classes
    cleantext = replace(cleantext,' class="MsoNormal"',"","ALL");

    cleantext = replace(cleantext,' class="cs_linkbar_item"',"","ALL");
    cleantext = replace(cleantext,' align="left"',"","ALL");

    // remove the follwing tags: font, span, div and script
    cleantext = REReplaceNoCase(cleantext,"<\s*\/?\s*font\s*.*?>","","ALL");
    cleantext = REReplaceNoCase(cleantext,"<\s*\/?\s*span\s*.*?>","","ALL");
    cleantext = REReplaceNoCase(cleantext,"<\s*\/?\s*div\s*.*?>","","ALL");
    cleantext = REReplaceNoCase(cleantext,"<script[^>]*>(?:[^<]+|<(?!/script>))+","","ALL");

    // this lower cases all stuff in tags
    //cleantext = REReplaceNoCase(cleantext,"<([^!].*?)>","\L<\1>","ALL");

    return cleantext;
  }
  </cfscript>

</cfcomponent>

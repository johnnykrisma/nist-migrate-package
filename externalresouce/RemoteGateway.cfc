<!--- Document Information -----------------------------------------------------
Title:    RemoteGateway.cfc
Author:   John Venable <jvenable@figleaf.com> heavily modified version of original by John Allen
Purpose:  I am the Remote Gateway. I am used to get the publications data from
      a remote dataprovider, the publications external database.
Usage:
Modification Log:
Name        Date      Description
================================================================================
John Allen      07/06/2008    Created
John Venable    07/15/2010    Modified to work without framework's "pageevent"
Doug Ward     07/26/2010    Updated "getSelectedPubs" function; pulled out references to FigFactor.
Gary Snodgrass    05/10/2011    Updated cfcatch outputs
Gary Snodgrass    05/11/2011    Set returntype for convertOracleOutput to "any"
Youchun Yao   05/18/2011    Added nike_admin.publication_vw_cs.active_status = 'A' check
------------------------------------------------------------------------------->
<cfcomponent displayname="Remote Gateway" hint="I am the Remote Gateway" output="false">

  <cfset variables.fakePublicationsQuery = 0 />
  <cfset variables.CommonspotDSN = "commonspot-nist" />
  <cfset variables.CommonspotSupportDSN = "commonspot-nist-custom" />
  <cfset variables.PublicationsDSN = "commonspot-source-taxonomy" />

  <!--- *********** Public ************ --->

  <!--- init --->
  <cffunction name="init" returntype="RemoteGateway" access="public" output="false"
    displayname="Init" hint="I am the constructor."
    description="I am the pseudo constructor for this CFC. I return an instance of myself.">

    <cfreturn this />
  </cffunction>

  <cffunction name="testString" output="false" returntype="string">
    <cfreturn "test">
  </cffunction>

  <cffunction name="getSelectedPubs" output="false" hint="I return selected publications." returntype="Query">
    <cfargument name="pubslist" type="string" required="yes" hint="I am the comma-delimited list of pubs to pull" />
    <cfargument name="returncount" type="numeric" hint="how many records should I return" default="6" />

    <cfset var qGetSelectedPubs = QueryNew("")>
    <cfset row = 0>
      <cftry>
      <cfquery name="qGetSelectedPubs" datasource="#variables.PublicationsDSN#">
        SELECT pub_id, title, date_published, abstract, last_modified
        FROM NIKE_ADMIN.PUBLICATION_VW_CS
        WHERE PUB_ID in (<cfqueryparam cfsqltype="cf_sql_integer" list="true" value="#arguments.pubslist#" />)
        AND active_status = 'A'
        ORDER BY CASE pub_id
        <cfloop index="i" list="#arguments.pubslist#">
          WHEN #i# THEN #row+1#
          <cfset row = row+1>
        </cfloop>
        END
      </cfquery>
      <cfcatch>
        <cfset qGetSelectedPubs = QueryNew("")>
        <cflog text="getSelectedPubs(qry): #cfcatch.Message# #cfcatch.Detail# Arguments: (#arguments.pubslist#) Trace: #mid(cfcatch.tagcontext[1].RAW_TRACE,find("(",cfcatch.tagcontext[1].RAW_TRACE), len(cfcatch.tagcontext[1].RAW_TRACE))# " type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
      </cfcatch>
      </cftry>

      <cfif qGetSelectedPubs.RecordCount>
        <cfloop from="1" to="#qGetSelectedPubs.recordcount#" index="x">
          <cftry>
            <cfset qGetSelectedPubs.title[x] = this.convertOracleOutput(qGetSelectedPubs.title[x]) />
            <cfcatch>
              <cflog text="getSelectedPubs(convertOracleOutput): #cfcatch.Message# #cfcatch.Detail# Pubslist: (#arguments.pubslist#) TRecordcount: #qGetSelectedPubs.recordcount#" type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
            </cfcatch>
          </cftry>
        </cfloop>
      </cfif>

    <cfreturn qGetSelectedPubs />
  </cffunction>

  <cffunction name="getRelatedOUPubs" output="false" hint="I return related publications.">
    <cfargument name="ou_ID" required="false" default="" />

    <cfset var publications = QueryNew("")>
      <cfset var x = 0 />

    <cftry>
       <cfquery name="publications" datasource="#variables.PublicationsDSN#">
       SELECT * FROM
      (
         SELECT pub_id, title, date_published, abstract, last_modified, count(*) over() AS allrecordscount
         FROM NIKE_ADMIN.PUBLICATION_VW_CS
         WHERE OP_UNIT in (<cfqueryparam cfsqltype="cf_sql_integer" value="#arguments.ou_ID#" />)
         AND DATE_PUBLISHED IS NOT NULL
         AND active_status = 'A'
         ORDER BY DATE_PUBLISHED desc
        ) WHERE rownum <= 20
       </cfquery>
    <cfcatch>
      <cfset publications = QueryNew("")>
      <cflog text="getRelatedOUPubs(qry): #cfcatch.Message# #cfcatch.Detail# Arguments: (#arguments.ou_ID#) Trace: #mid(cfcatch.tagcontext[1].RAW_TRACE,find("(",cfcatch.tagcontext[1].RAW_TRACE), len(cfcatch.tagcontext[1].RAW_TRACE))#" type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
    </cfcatch>
    </cftry>

    <cfif isQuery(publications)>
      <cfloop from="1" to="#publications.recordcount#" index="x">
        <cftry>
          <cfset publications.title[x] = this.convertOracleOutput(publications.title[x])/>
        <cfcatch>
          <cflog text="getRelatedOUPubs(convertOracleOutput): #cfcatch.Message# #cfcatch.Detail# Recordcount: #qGetSelectedPubs.recordcount#" type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
        </cfcatch>
        </cftry>
      </cfloop>
      </cfif>

    <cfreturn publications />
  </cffunction>

  <cffunction name="getPubsByMeta" output="false" hint="I return publications based on taxonomy term passed in." returntype="Query">
    <cfargument name="metaid" type="string" required="yes" hint="I am the metaID to pull" />
    <cfargument name="returncount" type="numeric" hint="how many records should I return" default="3" />

    <cfset var qGgetPubsByMeta = QueryNew("")>
      <cftry>
      <cfquery name="qGgetPubsByMeta" datasource="#variables.PublicationsDSN#">
        SELECT * FROM
          (
          SELECT p.pub_id, p.title, p.date_published, p.abstract, p.last_modified, p.OP_UNIT, p.DIVISION,
              p.GROUP_NUM, p.RESEARCH_FIELD, p.authorlist, p.keywords
          FROM NIKE_ADMIN.PUBLICATION_VW_CS p
          WHERE p.RESEARCH_FIELD LIKE (<cfqueryparam cfsqltype="CF_SQL_VARCHAR" value="%#arguments.metaid#%" />)
          AND p.DATE_PUBLISHED IS NOT NULL
          AND p.active_status = 'A'
          ORDER BY p.DATE_PUBLISHED DESC
          ) WHERE rownum <= #arguments.returncount#
      </cfquery>
      <cfcatch>
        <cfset qGgetPubsByMeta = QueryNew("")>
        <cflog text="getPubsByMeta(qry): #cfcatch.Message# #cfcatch.Detail# Arguments: (#arguments.metaid#) Trace: #mid(cfcatch.tagcontext[1].RAW_TRACE,find("(",cfcatch.tagcontext[1].RAW_TRACE), len(cfcatch.tagcontext[1].RAW_TRACE))#" type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
      </cfcatch>
      </cftry>

      <cfif isQuery(qGgetPubsByMeta)>
        <cfloop from="1" to="#qGgetPubsByMeta.recordcount#" index="x">
          <cftry>
            <cfset qGgetPubsByMeta.title[x] = this.convertOracleOutput(qGgetPubsByMeta.title[x]) />
          <cfcatch>
            <cflog text="getPubsByMeta(convertOracleOutput): #cfcatch.Message# #cfcatch.Detail# Recordcount: #qGgetPubsByMeta.recordcount#" type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
          </cfcatch>
          </cftry>
        </cfloop>
      </cfif>

    <cfreturn qGgetPubsByMeta />
  </cffunction>

  <cffunction name="getPubsByAuthorID" output="false" hint="retrieve publications based on an Author ID">
    <cfargument name="authorid" type="string" required="yes">
    <cfargument name="returncount" type="numeric" hint="how many records should I return. 0 means return all" default="3" />

    <cftry>
    <cfquery name="qGetPubsByAuthorID" datasource="#variables.PublicationsDSN#">
      <cfif arguments.returncount GT 0>SELECT * FROM (</cfif>
        SELECT p.pub_id, p.title, p.date_published, p.abstract, p.last_modified, p.OP_UNIT, p.DIVISION, p.GROUP_NUM,
          p.RESEARCH_FIELD, p.authorlist, p.keywords, count(*) over() AS total
        FROM NIKE_ADMIN.PUBLICATION_VW_CS p, NIKE_ADMIN.PUBLISHED_AUTHOR_VW_CS pa
        WHERE p.pub_id = pa.pub_id AND pa.author_id = <cfqueryparam cfsqltype="cf_sql_integer" list="yes" value="#arguments.authorid#">
        AND p.DATE_PUBLISHED IS NOT NULL
        AND p.active_status = 'A'
        ORDER BY p.DATE_PUBLISHED DESC
      <cfif arguments.returncount GT 0>)  WHERE ROWNUM <= #arguments.returncount#</cfif>
    </cfquery>
    <cfcatch>
      <cfset qGetPubsByAuthorID = QueryNew("")>
      <cflog text="getPubsByAuthorID(qry): #cfcatch.Message# #cfcatch.Detail# Arguments: (#arguments.authorid#) Trace: #mid(cfcatch.tagcontext[1].RAW_TRACE,find("(",cfcatch.tagcontext[1].RAW_TRACE), len(cfcatch.tagcontext[1].RAW_TRACE))#" type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
    </cfcatch>
    </cftry>

    <cfif isQuery(qGetPubsByAuthorID)>
      <cfloop from="1" to="#qGetPubsByAuthorID.recordcount#" index="x">
        <cftry>
          <cfset qGetPubsByAuthorID.title[x] = this.convertOracleOutput(qGetPubsByAuthorID.title[x]) />
          <cfcatch>
            <cflog text="getPubsByAuthorID(convertOracleOutput): #cfcatch.Message# #cfcatch.Detail# Recordcount: #qGetPubsByAuthorID.recordcount#" type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
          </cfcatch>
        </cftry>
      </cfloop>
    </cfif>

    <cfreturn qGetPubsByAuthorID />
  </cffunction>

  <cffunction name="GetLatestOUPubs" output="false" hint="I return an OU's related publications.">
    <cfargument name="ouid" hint="the ID of the OU" />

    <cftry>
    <cfquery name="qGetLatestOUPubs" datasource="#variables.PublicationsDSN#">
      SELECT * FROM
        (SELECT X.*,rownum as r FROM
          (
            SELECT pub_id, title, date_published, abstract, last_modified, count(*) over() AS allrecordscount
            FROM NIKE_ADMIN.PUBLICATION_VW_CS
            WHERE OP_UNIT in <cfqueryparam cfsqltype="cf_sql_integer" value="#arguments.ouid#" />
            AND DATE_PUBLISHED IS NOT NULL
            AND active_status = 'A'
            ORDER BY DATE_PUBLISHED desc
          ) X
        )
      WHERE r <= 20
    </cfquery>
    <cfcatch>
      <cfset qGetLatestOUPubs = QueryNew("")>
      <cflog text="GetLatestOUPubs(qry): #cfcatch.Message# #cfcatch.Detail# Arguments: (#arguments.ouid#) Trace: #mid(cfcatch.tagcontext[1].RAW_TRACE,find("(",cfcatch.tagcontext[1].RAW_TRACE), len(cfcatch.tagcontext[1].RAW_TRACE))#" type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
    </cfcatch>
    </cftry>

    <cfif isQuery(qGetLatestOUPubs)>
      <cfloop from="1" to="#qGetLatestOUPubs.recordcount#" index="x">
        <cftry>
          <cfset publications.title[x] = this.convertOracleOutput(publications.title[x])/>
          <cfcatch>
            <cflog text="GetLatestOUPubs(convertOracleOutput): #cfcatch.Message# #cfcatch.Detail# Recordcount: #qGetLatestOUPubs.recordcount#" type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
          </cfcatch>
        </cftry>
      </cfloop>
      </cfif>

    <cfreturn qGetLatestOUPubs />
  </cffunction>

  <!--- getDivisionPubs --->
  <cffunction name="getDivisionPubs" output="false" hint="I return a Divisions selected publications.">

    <cfargument name="division_id" required="true" default="" />

    <cfset var publications = 0 />
      <cfset var x = 0 />

    <cfif structKeyExists(arguments.pageEvent.getAllValues().customElement, "division_id")
      and
      len(arguments.pageEvent.getAllValues().customElement.division_id)>
      <cftry>
      <cfquery name="publications" datasource="#variables.PublicationsDSN#">
        SELECT * FROM
          (SELECT X.*,rownum as r FROM
            (
              SELECT pub_id, title, date_published, abstract, last_modified, count(*) over() AS allrecordscount
              FROM NIKE_ADMIN.PUBLICATION_VW_CS
              WHERE DIVISION in (<cfqueryparam cfsqltype="cf_sql_integer" list="true" value="#arguments.pageevent.getAllValues().customElement.division_id#" />)
              AND DATE_PUBLISHED IS NOT NULL
              AND active_status = 'A'
              ORDER BY DATE_PUBLISHED desc
            ) X
          )
        WHERE r <= 3
      </cfquery>
      <cfcatch>
        <cfset publications = QueryNew("")>
        <cflog text="getDivisionPubs(qry): #cfcatch.Message# #cfcatch.Detail# Arguments: (#arguments.division_id#) Trace: #mid(cfcatch.tagcontext[1].RAW_TRACE,find("(",cfcatch.tagcontext[1].RAW_TRACE), len(cfcatch.tagcontext[1].RAW_TRACE))#" type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
      </cfcatch>
      </cftry>
    </cfif>

      <cfif isQuery(publications)>
      <cfloop from="1" to="#publications.recordcount#" index="x">
        <cftry>
          <cfset publications.title[x] = this.convertOracleOutput(publications.title[x])/>
          <cfcatch>
            <cflog text="getDivisionPubs(convertOracleOutput): #cfcatch.Message# #cfcatch.Detail# Recordcount: #publications.recordcount#" type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
          </cfcatch>
        </cftry>
      </cfloop>
      </cfif>

    <cfreturn publications />
  </cffunction>

  <!--- getGroupPublications --->
  <cffunction name="getGroupPublications" output="false" hint="I return a Groups latest publications.">

    <cfargument name="pageEvent" hint="I am the PageEvent object.<br />I am required." />

    <cfset var publications = 0 />
      <cfset var x = 0 />

    <cfif structKeyExists(arguments.pageEvent.getAllValues().customElement, "group_id")
      and
      len(arguments.pageEvent.getAllValues().customElement.group_id)>
      <cftry>
      <cfquery name="publications" datasource="#variables.PublicationsDSN#" blockfactor="3">
        SELECT * FROM
          (SELECT X.*,rownum as r FROM
            (
              SELECT pub_id, title, date_published, abstract, last_modified, count(*) over() AS allrecordscount
              FROM NIKE_ADMIN.PUBLICATION_VW_CS
              WHERE GROUP_NUM = <cfqueryparam value="#arguments.pageevent.getAllValues().customElement.group_id#" />
              <cfif structKeyExists(arguments.pageEvent.getAllValues().customElement, "group_division_id")>
                AND DIVISION = <cfqueryparam cfsqltype="cf_sql_integer" value="#arguments.pageEvent.getAllValues().customElement.group_division_id#" />
              </cfif>
              AND DATE_PUBLISHED IS NOT NULL
              AND active_status = 'A'
              order by DATE_PUBLISHED desc
            ) X
          )
        WHERE r <= 3
      </cfquery>
      <cfcatch>
        <cfset publications = QueryNew("")>
        <cflog text="getGroupPublications(qry): #cfcatch.Message# #cfcatch.Detail# Arguments: (#arguments.pageEvent.getAllValues().customElement.group_id#) Trace: #mid(cfcatch.tagcontext[1].RAW_TRACE,find("(",cfcatch.tagcontext[1].RAW_TRACE), len(cfcatch.tagcontext[1].RAW_TRACE))#" type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
      </cfcatch>
      </cftry>
    </cfif>

      <cfif isQuery(publications)>
        <cftry>
        <cfloop from="1" to="#publications.recordcount#" index="x">
          <cfset publications.title[x] = this.convertOracleOutput(publications.title[x])/>
        </cfloop>
        <cfcatch>
          <cflog text="getGroupPublications(convertOracleOutput): #cfcatch.Message# #cfcatch.Detail# Recordcount: #publications.recordcount#" type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
        </cfcatch>
      </cftry>
      </cfif>

    <cfreturn publications />
  </cffunction>

  <!--- ********************************* Methods to export data into Drupal ********************************* --->

  <cffunction name="getPubFiles" access="remote" output="false" returnType="array" returnFormat="json">
    <cfargument name="pid" displayname="Publication ID" required="false" type="numeric" />

    <cfcontent type="application/json">

    <cftry>

      <cfquery name="qGetPubIDs" datasource="#variables.PublicationsDSN#" blockfactor="3" cachedwithin="0.0416666666667">
        SELECT p.pub_id
          <cfif structKeyExists(arguments, "pid")>,TO_CHAR(f.UPLOADED_DT, 'YYYY-MM-DD HH24:MI') AS created,
          TO_CHAR(f.LAST_MODIFIED_DT, 'YYYY-MM-DD HH24:MI') AS changed,
          'http://www.nist.gov/customcf/get_pdf.cfm?pub_id=' || p.pub_id AS url</cfif>
        FROM NIKE_ADMIN.PUBLICATION_VW_CS p JOIN NIKE_ADMIN.PUB_FILE_INFO_VW_CS f
        ON p.pub_id = f.pub_id
        <cfif structKeyExists(arguments, "pid")>
          AND p.pub_id = <cfqueryparam cfsqltype="cf_sql_integer" value="#arguments.pid#">
        </cfif>
        ORDER BY f.LAST_MODIFIED_DT DESC
      </cfquery>

      <cfreturn queryToArray(qGetPubIDs)>


    <cfcatch>
      <cflog text="RemoteGateway Error: getPubFileIDs: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn  arrayNew("")>
    </cfcatch>

    </cftry>
</cffunction>


  <cffunction name="getPubIDs" access="remote" output="false" returnType="array" returnFormat="json">
    <cfargument name="start" displayname="Which record to start with for paging" required="no" type="numeric" default="1">
    <cfargument name="num" displayname="How many records to fetch per page" required="no" type="numeric" default="100000">

    <cfparam name="startrecord" default="1">
    <cfparam name="endrecord" default="50">

    <cfif structKeyExists(arguments, "start")>
      <cfset var startrecord = arguments.start>
      <cfset var endrecord = arguments.start + arguments.num>
    </cfif>

     <cfcontent type="application/json">

    <cftry>

      <cfquery name="qGetPubIDs" datasource="#variables.PublicationsDSN#" cachedwithin="0">
        SELECT pub_id AS id
        FROM NIKE_ADMIN.PUBLICATION_VW_CS
        ORDER BY last_modified DESC
        <!---
        SELECT *
        FROM (
          SELECT ROWNUM rnum ,a.* FROM (
            SELECT pub_id AS id, TO_CHAR(last_modified, 'YYYY-MM-DD HH24:MM:SS') as last_modified
            FROM NIKE_ADMIN.PUBLICATION_VW_CS
            ORDER BY last_modified DESC
          )
        a)
        WHERE rnum BETWEEN <cfqueryparam cfsqltype="cf_sql_integer" value="#startrecord#"> AND <cfqueryparam cfsqltype="cf_sql_integer" value="#endrecord#">
        --->
      </cfquery>

      <cfreturn queryToArray(qGetPubIDs)>

    <cfcatch>
      <cflog text="RemoteGateway Error: getPubIDs: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
    </cfcatch>

    </cftry>
</cffunction>










   <cffunction name="getPubFromIDStruct" access="remote" output="false" returnType="any" returnFormat="json">
    <cfargument name="pid" displayname="Publication ID" required="no" type="numeric">

    <cfcontent type="application/json">
    <cftry>

      <cfquery name="qGetPubFromID" dataSource="#variables.PublicationsDSN#" cachedwithin="0">
          SELECT p.pub_id, p.division, p.title, p.project_name, p.project_code, p.keywords, p.abstract, p.group_name, p.group_num, p.op_unit,
            to_char(p.date_published, 'YYYY-MM-DD HH24:MM:SS') as pubdate, TO_CHAR(last_modified, 'YYYY-MM-DD HH24:MM:SS') as last_modified, p.pub_type,
            p.research_field, p.chapter, p.volume, p.issue, p.start_page, p.end_page, p.num_pages, p.pub_report_num, p.doi, p.filesize,
            CASE NIST_ID
            WHEN -1 THEN a.author_first ||' ' || upper(substr(a.author_middle,1,1)) || nvl2(a.author_middle, '.', '') ||' '|| a.author_last || ', '
            ELSE '<span class="nist-author">' || a.author_first ||' ' || upper(substr(a.author_middle,1,1)) || nvl2(a.author_middle, '.', '') ||' '|| a.author_last || '</span>, '
            END AS author_name
          FROM NIKE_ADMIN.PUBLICATION_VW_CS p JOIN NIKE_ADMIN.PUBLISHED_AUTHOR_VW_CS pa ON p.pub_id = pa.pub_id
          JOIN NIKE_ADMIN.AUTHOR_VW_CS a ON a.author_id = pa.author_id
          WHERE p.active_status = 'A'
          <cfif structKeyExists(arguments, "pid")>
            AND p.pub_id = <cfqueryparam cfsqltype="cf_sql_integer" value="#arguments.pid#">
          </cfif>
          ORDER BY p.pub_id, pa.author_order, last_modified DESC
      </cfquery>
      <!--- Additional Fields required for various pub types lots of business logic centered around pub_type
        0 pub_type Journal
          publication_title (as citation)
        1 pub_type Book
          book_title (as citation), publisher_name, publisher_address, publisher_city, publisher_state, publisher_country
        2 pub_type Encyclopaedia
          publication_title (as citation)
        3 pub_type Conference
          proceedings_title (as citation), conference_title, city, state, country, conf_begin, conf_end
        4 pub_type Web
          publication_title (as citation), link
        5 pub_type NIST
          publication_title (as citation)
        6 pub_type Other
          other_title (as citation)
        7 pub_type Book Chapter
          book_title (as citation), publisher_name, publisher_address, publisher_city, publisher_state, publisher_country
      --->

      <!---
      <cfset pubquery = QueryNew("abstract, author_list, citation, conf_dates, conf_location, conf_title, filesize, doi, issue, keywords, last_modified, pages, org_unit, proceedings_title, id, pubdate, pubtype, publisher_info, report_num, research_field, series, title, volume, web_link", "varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, integer, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar")>
      --->
      <cfset pubStruct = StructNew()>
      <cfoutput query="qGetPubFromID" group="pub_id">
        <cfif len(trim(qGetPubFromID.research_field))>
        	<!--- research areas are stored in the DB as semicolon-delimited lists!! convert it to comma-delimited for an IN clause --->
        	<cfset researchAreaList = ListChangeDelims(qGetPubFromID.research_field, ",", ";")>
        	<cftry><!--- ran into some issues where the research_field just says 'error' --->
        		<cfquery name="getPubResearchAreas" datasource="#variables.PublicationsDSN#">
        			SELECT level_name as area
        			FROM NIKE_ADMIN.X_LEVEL_NAME_VW_CS ln, NIKE_ADMIN.X_TAXONOMY_LEVEL_VW_CS ll
        			WHERE ll.level_name_id=ln.level_name_id AND ll.level_id IN (#researchAreaList#)
        		</cfquery>
        		<cfset researchAreas = valuelist(getPubResearchAreas.area, ",&nbsp;")>
        	<cfcatch>
        		<cfset researchAreas = "error">
        	</cfcatch>
        	</cftry>
        <cfelse>
        	<cfset researchAreas = "norecords">
        </cfif>

        <cfset tempAuthorList = "">
        <cfoutput>
          <cfset tempAuthorList = tempAuthorList & author_name & " ">

        </cfoutput>
        <cfset StructInsert(pubStruct, "author_list", tempAuthorList)>
        <cfset StructInsert(pubStruct, "abstract", convertOracleOutput(abstract))>
        <cfset StructInsert(pubStruct, "citation", "")>
        <cfset StructInsert(pubStruct, "conf_dates", "")>
        <cfset StructInsert(pubStruct, "conf_location", "")>
        <cfset StructInsert(pubStruct, "conf_title", "")>
        <cfif len(trim(doi))>
          <cfset StructInsert(pubStruct, "doi", "http://dx.doi.org/" & doi)>
        <cfelse>
          <cfset StructInsert(pubStruct, "doi", doi)>
        </cfif>
        <cfset StructInsert(pubStruct, "filesize", filesize)>
        <cfset StructInsert(pubStruct, "issue", issue)>
        <cfset StructInsert(pubStruct, "keywords", convertOracleOutput(keywords))>
        <cfset StructInsert(pubStruct, "last_modified", last_modified)>
        <cfset StructInsert(pubStruct, "org_unit", group_name)>
        <cfset StructInsert(pubStruct, "pages", num_pages)>
        <cfset StructInsert(pubStruct, "start_page", start_page)>
        <cfset StructInsert(pubStruct, "end_page", end_page)>

        <cfset StructInsert(pubStruct, "proceedings_title", "")>
        <cfset StructInsert(pubStruct, "id", pub_id)>
        <cfset StructInsert(pubStruct, "pubtype", convertPubType(pub_type))>

        <cfif left(pubdate,2) EQ "00"><!--- stupid broked data with years like 0015 --->
          <cfset StructInsert(pubStruct, "pubdate", replace(pubdate, "00", "20", "one"))>
        <cfelse>
          <cfset StructInsert(pubStruct, "pubdate", pubdate)>
        </cfif>

        <cfset StructInsert(pubStruct, "publisher_info", "")>
        <cfset StructInsert(pubStruct, "research_field", researchAreas)>
        <cfset StructInsert(pubStruct, "series", "")>
        <cfset StructInsert(pubStruct, "title", left(convertOracleOutput(title),255))>
        <cfset StructInsert(pubStruct, "volume", volume)>
        <cfset StructInsert(pubStruct, "web_link", "")>
      </cfoutput>

      <!--- lots of business logic centered around pub_type --->
      <cfswitch expression="#qGetPubFromID.pub_type#">
        <cfcase value="0"><!--- pub_type Journal --->
          <cfset pubStruct.citation = convertOracleOutput(this.getJournalInfo(qGetPubFromID.pub_id).citation)>
        </cfcase><!--- END Journal --->

        <cfcase value="1"><!--- pub_type Book --->
          <cfset pubStruct.citation = convertOracleOutput(this.getBookInfo(qGetPubFromID.pub_id).citation)>
          <cfset pubStruct.publisher_info = this.getBookInfo(qGetPubFromID.pub_id).publisher_info>
        </cfcase><!--- END Book --->

        <cfcase value="2"><!--- pub_type Encyclopaedia --->
          <cfset pubStruct.citation = convertOracleOutput(this.getEncyclopediaInfo(qGetPubFromID.pub_id).citation)>
        </cfcase><!--- END Encyclopaedia --->

        <cfcase value="3"><!--- pub_type Conference --->
           <cfset pubStruct.conf_location = this.getConferenceInfo(qGetPubFromID.pub_id).conf_location>
           <cfset pubStruct.conf_dates = this.getConferenceInfo(qGetPubFromID.pub_id).conf_dates>
           <cfset pubStruct.conf_title = this.getConferenceInfo(qGetPubFromID.pub_id).conf_title>
           <cfset pubStruct.proceedings_title = this.getConferenceInfo(qGetPubFromID.pub_id).proceedings_title>
        </cfcase><!--- END Conference --->

        <cfcase value="4"><!--- pub_type Web --->
          <cfset pubStruct.citation = convertOracleOutput(this.getWebsiteInfo(qGetPubFromID.pub_id).citation)>
          <cfset pubStruct.web_link = this.getWebsiteInfo(qGetPubFromID.pub_id).web_link>
        </cfcase><!--- END Web --->

        <cfcase value="5"><!--- pub_type NIST --->
          <cfset pubStruct.citation = convertOracleOutput(this.getNistInfo(qGetPubFromID.pub_id).citation)>
        </cfcase><!--- END NIST --->

        <cfcase value="6"><!--- pub_type Other --->
          <cfset pubStruct.citation = convertOracleOutput(this.getOtherInfo(qGetPubFromID.pub_id).citation)>
        </cfcase><!--- END Other --->

        <cfcase value="7"><!--- pub_type Book Chapter --->
          <cfset pubStruct.citation = convertOracleOutput(this.getBookChapterInfo(qGetPubFromID.pub_id).citation)>
          <cfset pubStruct.publisher_info = this.getBookChapterInfo(qGetPubFromID.pub_id).publisher_info>
        </cfcase><!--- END Book Chapter --->

      </cfswitch>

      <cfreturn pubStruct>

    <cfcatch>
      <cflog text="RemoteGateway Error: getPubFromIDStruct: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn  arrayNew("")>
    </cfcatch>

    </cftry>
  </cffunction>


  <cffunction name="getPubFromID" access="remote" output="false" returnType="array" returnFormat="json">
    <cfargument name="pid" displayname="Publication ID" required="no" type="numeric">

    <cfcontent type="application/json">
    <cftry>

      <cfquery name="qGetPubFromID" dataSource="#variables.PublicationsDSN#" cachedwithin="0">
          SELECT p.pub_id, p.division, p.title, p.project_name, p.project_code, p.keywords, p.abstract, p.group_name, p.group_num, p.op_unit,
            to_char(p.date_published, 'YYYY-MM-DD HH24:MM:SS') as pubdate, TO_CHAR(last_modified, 'YYYY-MM-DD HH24:MM:SS') as last_modified, p.pub_type,
            p.research_field, p.chapter, p.volume, p.issue, p.start_page, p.end_page, p.num_pages, p.pub_report_num, p.doi, p.filesize,
            CASE NIST_ID
            WHEN -1 THEN a.author_first ||' ' || upper(substr(a.author_middle,1,1)) || nvl2(a.author_middle, '.', '') ||' '|| a.author_last || ', '
            ELSE '<span class="nist-author">' || a.author_first ||' ' || upper(substr(a.author_middle,1,1)) || nvl2(a.author_middle, '.', '') ||' '|| a.author_last || '</span>, '
            END AS author_name
          FROM NIKE_ADMIN.PUBLICATION_VW_CS p JOIN NIKE_ADMIN.PUBLISHED_AUTHOR_VW_CS pa ON p.pub_id = pa.pub_id
          JOIN NIKE_ADMIN.AUTHOR_VW_CS a ON a.author_id = pa.author_id
          WHERE p.active_status = 'A'
          <cfif structKeyExists(arguments, "pid")>
            AND p.pub_id = <cfqueryparam cfsqltype="cf_sql_integer" value="#arguments.pid#">
          </cfif>
          ORDER BY p.pub_id, pa.author_order, last_modified DESC
      </cfquery>
      <!--- Additional Fields required for various pub types lots of business logic centered around pub_type
        0 pub_type Journal
          publication_title (as citation)
        1 pub_type Book
          book_title (as citation), publisher_name, publisher_address, publisher_city, publisher_state, publisher_country
        2 pub_type Encyclopaedia
          publication_title (as citation)
        3 pub_type Conference
          proceedings_title (as citation), conference_title, city, state, country, conf_begin, conf_end
        4 pub_type Web
          publication_title (as citation), link
        5 pub_type NIST
          publication_title (as citation)
        6 pub_type Other
          other_title (as citation)
        7 pub_type Book Chapter
          book_title (as citation), publisher_name, publisher_address, publisher_city, publisher_state, publisher_country
      --->

      <cfset pubquery = QueryNew("abstract, author_list, citation, conf_dates, conf_location, conf_title, filesize, doi, issue, keywords, last_modified, pages, org_unit, proceedings_title, id, pubdate, pubtype, publisher_info, report_num, research_field, series, title, volume, web_link", "varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, integer, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar, varchar")>

      <cfoutput query="qGetPubFromID" group="pub_id">
        <cfset temp = QueryAddRow(pubquery)>
        <cfset tempAuthorList = "">
        <cfoutput>
          <cfset tempAuthorList = tempAuthorList & author_name & " ">
          <cfset Temp = QuerySetCell(pubquery, "author_list", tempAuthorList)>
        </cfoutput>
        <cfset Temp = QuerySetCell(pubquery, "abstract", convertOracleOutput(abstract))>
        <cfset Temp = QuerySetCell(pubquery, "citation", "")>
        <cfset Temp = QuerySetCell(pubquery, "conf_dates", "")>
        <cfset Temp = QuerySetCell(pubquery, "conf_location", "")>
        <cfset Temp = QuerySetCell(pubquery, "conf_title", "")>
        <cfif len(trim(doi))>
          <cfset Temp = QuerySetCell(pubquery, "doi", "http://dx.doi.org/" & doi)>
        <cfelse>
          <cfset Temp = QuerySetCell(pubquery, "doi", doi)>
        </cfif>
        <cfset Temp = QuerySetCell(pubquery, "filesize", filesize)>
        <cfset Temp = QuerySetCell(pubquery, "issue", issue)>
        <cfset Temp = QuerySetCell(pubquery, "keywords", convertOracleOutput(keywords))>
        <cfset Temp = QuerySetCell(pubquery, "last_modified", last_modified)>
        <cfset Temp = QuerySetCell(pubquery, "org_unit", group_name)>
        <cfset Temp = QuerySetCell(pubquery, "pages", num_pages)>
        <cfset Temp = QuerySetCell(pubquery, "proceedings_title", "")>
        <cfset Temp = QuerySetCell(pubquery, "id", pub_id)>
        <cfset Temp = QuerySetCell(pubquery, "pubtype", convertPubType(pub_type))>
        <cfset Temp = QuerySetCell(pubquery, "pubdate", pubdate)>
        <cfset Temp = QuerySetCell(pubquery, "publisher_info", "")>
        <cfset Temp = QuerySetCell(pubquery, "research_field", research_field)>
        <cfset Temp = QuerySetCell(pubquery, "series", "")>
        <cfset Temp = QuerySetCell(pubquery, "title", convertOracleOutput(title))>
        <cfset Temp = QuerySetCell(pubquery, "volume", volume)>
        <cfset Temp = QuerySetCell(pubquery, "web_link", "")>
      </cfoutput>

      <!--- lots of business logic centered around pub_type --->
      <cfswitch expression="#qGetPubFromID.pub_type#">
        <cfcase value="0"><!--- pub_type Journal --->
          <cfset pubquery.citation = convertOracleOutput(this.getJournalInfo(qGetPubFromID.pub_id).citation)>
        </cfcase><!--- END Journal --->

        <cfcase value="1"><!--- pub_type Book --->
          <cfset pubquery.citation = convertOracleOutput(this.getBookInfo(qGetPubFromID.pub_id).citation)>
          <cfset pubquery.publisher_info = this.getBookInfo(qGetPubFromID.pub_id).publisher_info>
        </cfcase><!--- END Book --->

        <cfcase value="2"><!--- pub_type Encyclopaedia --->
          <cfset pubquery.citation = convertOracleOutput(this.getEncyclopediaInfo(qGetPubFromID.pub_id).citation)>
        </cfcase><!--- END Encyclopaedia --->

        <cfcase value="3"><!--- pub_type Conference --->
           <cfset pubquery.conf_location = this.getConferenceInfo(qGetPubFromID.pub_id).conf_location>
           <cfset pubquery.conf_dates = this.getConferenceInfo(qGetPubFromID.pub_id).conf_dates>
           <cfset pubquery.conf_title = this.getConferenceInfo(qGetPubFromID.pub_id).conf_title>
           <cfset pubquery.proceedings_title = this.getConferenceInfo(qGetPubFromID.pub_id).proceedings_title>
        </cfcase><!--- END Conference --->

        <cfcase value="4"><!--- pub_type Web --->
          <cfset pubquery.citation = convertOracleOutput(this.getWebsiteInfo(qGetPubFromID.pub_id).citation)>
          <cfset pubquery.web_link = this.getWebsiteInfo(qGetPubFromID.pub_id).web_link>
        </cfcase><!--- END Web --->

        <cfcase value="5"><!--- pub_type NIST --->
          <cfset pubquery.citation = convertOracleOutput(this.getNistInfo(qGetPubFromID.pub_id).citation)>
          <cfset pubquery.citation = this.getNistInfo(qGetPubFromID.pub_id).series>
        </cfcase><!--- END NIST --->

        <cfcase value="6"><!--- pub_type Other --->
          <cfset pubquery.citation = convertOracleOutput(this.getOtherInfo(qGetPubFromID.pub_id).citation)>
        </cfcase><!--- END Other --->

        <cfcase value="7"><!--- pub_type Book Chapter --->
          <cfset pubquery.citation = convertOracleOutput(this.getBookChapterInfo(qGetPubFromID.pub_id).citation)>
          <cfset pubquery.publisher_info = this.getBookChapterInfo(qGetPubFromID.pub_id).publisher_info>
        </cfcase><!--- END Book Chapter --->

      </cfswitch>

      <cfreturn queryToArray(pubquery)>

    <cfcatch>
      <cflog text="RemoteGateway Error: getPubFromID: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfreturn  arrayNew("")>
    </cfcatch>

    </cftry>
  </cffunction>

  <!--- * * * * * * * * * * * * * * * * * * * * * 0 JOURNAL INFORMATION  * * * * * * * * * * * * * * * * * * * * * --->
  <cffunction name="getJournalInfo" access="remote" output="false" returnType="any">
    <cfargument name="pid" displayname="Publication ID" required="yes" type="numeric">

    <cfset sRetVal = structNew()>

    <cftry>

      <cfquery name="qGetJournalInfo" datasource="#variables.PublicationsDSN#">
        SELECT publication_title
        FROM NIKE_ADMIN.JOURNAL_VW_CS
        WHERE pub_id = <cfqueryparam cfsqltype="cf_sql_integer" value="#arguments.pid#">
      </cfquery>

      <cfscript>
        if(qGetJournalInfo.recordcount){
          sRetVal.citation = qGetJournalInfo.publication_title;
        } else {
          sRetVal.citation = "";
        }
      </cfscript>

    <cfcatch>
      <cflog text="RemoteGateway Error: getJournalInfo: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
      <cfset sRetVal.citation = "">
    </cfcatch>

    </cftry>

    <cfreturn sRetVal>
  </cffunction>

  <!--- * * * * * * * * * * * * * * * * * * * * * 1 BOOK INFORMATION  * * * * * * * * * * * * * * * * * * * * * --->
  <cffunction name="getBookInfo" access="remote" output="false" returnType="any">
    <cfargument name="pid" displayname="Publication ID" required="yes" type="numeric">

    <cfset sRetVal = structNew()>

    <cftry>

      <cfquery name="qGetBookInfo" datasource="#variables.PublicationsDSN#">
        SELECT book_title, publisher_name, publisher_address, publisher_city, publisher_state, publisher_country
        FROM NIKE_ADMIN.BOOK_VW_CS
        WHERE pub_id = <cfqueryparam cfsqltype="cf_sql_integer" value="#arguments.pid#">
      </cfquery>

      <cfscript>
        if(qGetBookInfo.recordcount){
          if(len(trim(qGetBookInfo.publisher_city))){
    				variables.book_loc = qGetBookInfo.publisher_city & ", ";
    			}
    			if(qGetBookInfo.publisher_state EQ "1" ) {
    				variables.book_loc = variables.book_loc & qGetBookInfo.publisher_country;
    			} else {
    				variables.book_loc = variables.book_loc & qGetBookInfo.publisher_state;
    			}

    			sRetVal.citation = qGetBookInfo.book_title;
    			sRetVal.publisher_info = qGetBookInfo.publisher_name & ", " & variables.book_loc;
        } else {
          sRetVal.citation = "";
          sRetVal.publisher_info = "";
        }
      </cfscript>

      <cfcatch>
        <cflog text="RemoteGateway Error: getBookInfo: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
        <cfset sRetVal.citation = "">
      </cfcatch>

      </cftry>

      <cfreturn sRetVal>
  </cffunction>

  <!--- * * * * * * * * * * * * * * * * * * * * * 2 ENCYCLOPAEDIA INFORMATION  * * * * * * * * * * * * * * * * * * * * * --->
  <cffunction name="getEncyclopediaInfo" access="remote" output="false" returnType="any">
    <cfargument name="pid" displayname="Publication ID" required="yes" type="numeric">

    <cfset sRetVal = structNew()>

    <cftry>

      <cfquery name="qGetEncyclopediaInfo" datasource="#variables.PublicationsDSN#">
        SELECT publication_title
        FROM NIKE_ADMIN.ENCYCLOPEDIA_VW_CS
        WHERE pub_id = <cfqueryparam cfsqltype="cf_sql_integer" value="#arguments.pid#">
      </cfquery>

      <cfscript>
        if(qGetEncyclopediaInfo.recordcount){
          sRetVal.citation = qGetEncyclopediaInfo.publication_title;
        } else {
          sRetVal.citation = "";
        }
      </cfscript>

      <cfcatch>
        <cflog text="RemoteGateway Error: getEncyclopediaInfo: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
        <cfset sRetVal.citation = "">
      </cfcatch>

      </cftry>

      <cfreturn sRetVal>
  </cffunction>

  <!--- * * * * * * * * * * * * * * * * * * * * 3 CONFERENCE INFORMATION * * * * * * * * * * * * * * * * * * * * --->
  <cffunction name="getConferenceInfo" access="remote" output="false" returnType="any">
    <cfargument name="pid" displayname="Publication ID" required="yes" type="numeric">

    <cfset sRetVal = structNew()>

    <cftry>

      <cfquery name="qGetConferenceInfo" datasource="#variables.PublicationsDSN#">
        SELECT proceedings_title, conference_title, city, state, country, conf_begin, conf_end
        FROM NIKE_ADMIN.CONFERENCE_VW_CS
        WHERE pub_id = <cfqueryparam cfsqltype="cf_sql_integer" value="#arguments.pid#">
      </cfquery>

      <cfif qGetConferenceInfo.recordcount>
        <cfscript>
          param name="variables.conference_location" default="";
          if (len(trim(qGetConferenceInfo.city))) {
            variables.conference_location = qGetConferenceInfo.city & ", ";
          }
          if ( qGetConferenceInfo.state EQ "1" ) {
            variables.conference_location = variables.conference_location &  qGetConferenceInfo.country;
          } else {
            variables.conference_location = variables.conference_location & qGetConferenceInfo.state;
          }
          sRetVal.conf_location = variables.conference_location;

          if ( len(trim(qGetConferenceInfo.conf_begin)) AND len(trim(qGetConferenceInfo.conf_end)) ) {
            sRetVal.conf_dates = application.fl_udf.DateRangeFormat(qGetConferenceInfo.conf_begin, qGetConferenceInfo.conf_end);
          } else if ( len(trim(qGetConferenceInfo.conf_begin)) AND NOT len(trim(qGetConferenceInfo.conf_end)) ) {
            sRetVal.conf_dates = application.fl_udf.DateRangeFormat(qGetConferenceInfo.conf_begin);
          } else {
            sRetVal.conf_dates = "";
          }

          if ( len(trim(qGetConferenceInfo.conference_title)) NEQ len(trim(qGetConferenceInfo.proceedings_title)) ) {
            sRetVal.conf_title = qGetConferenceInfo.conference_title;
            sRetVal.proceedings_title = qGetConferenceInfo.proceedings_title;
          } else {
            sRetVal.conf_title = qGetConferenceInfo.conference_title;
            sRetVal.proceedings_title = "";
          }
        </cfscript>
      <cfelse>
        <cfscript>
            sRetVal.conf_location = "";
            sRetVal.conf_dates = "";
            sRetVal.conf_title = "";
        </cfscript>
      </cfif>

      <cfcatch>
        <cflog text="RemoteGateway Error: getConferenceInfo: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
        <cfset sRetVal.citation = "">
      </cfcatch>

      </cftry>

    <cfreturn sRetVal>
  </cffunction>

  <!--- * * * * * * * * * * * * * * * * * * * * * 4 WEB INFORMATION  * * * * * * * * * * * * * * * * * * * * * --->
  <cffunction name="getWebsiteInfo" access="remote" output="false" returnType="any">
    <cfargument name="pid" displayname="Publication ID" required="yes" type="numeric">

    <cfset sRetVal = structNew()>

    <cftry>

      <cfquery name="qGetWebsiteInfo" datasource="#variables.PublicationsDSN#">
        SELECT publication_title, link
        FROM NIKE_ADMIN.WEBSITE_VW_CS
        WHERE pub_id = <cfqueryparam cfsqltype="cf_sql_integer" value="#arguments.pid#">
      </cfquery>

      <cfscript>
        if(qGetWebsiteInfo.recordcount){
          sRetVal.citation = qGetWebsiteInfo.publication_title;
          sRetVal.web_link = qGetWebsiteInfo.link;
        } else {
          sRetVal.citation = "";
          sRetVal.web_link = "";
        }
      </cfscript>

      <cfcatch>
        <cflog text="RemoteGateway Error: getWebsiteInfo: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
        <cfset sRetVal.citation = "">
      </cfcatch>
    </cftry>
    <cfreturn sRetVal>
  </cffunction>

  <!--- * * * * * * * * * * * * * * * * * * * * * 5 NIST INFORMATION  * * * * * * * * * * * * * * * * * * * * * --->
  <cffunction name="getNistInfo" access="remote" output="false" returnType="any">
    <cfargument name="pid" displayname="Publication ID" required="yes" type="numeric">

    <cftry>

      <cfquery name="qGetNistInfo" datasource="#variables.PublicationsDSN#">
        SELECT publication_title, p.pub_report_num
        FROM NIKE_ADMIN.NIST_PUB_VW_CS n JOIN NIKE_ADMIN.PUBLICATION_VW_CS p
        ON n.pub_id = p.pub_id
        WHERE n.pub_id = <cfqueryparam cfsqltype="cf_sql_integer" value="#arguments.pid#">
      </cfquery>

      <cfscript>
			if(qGetNistInfo.recordcount){
				sRetVal.citation = convertNistPub(qGetNistInfo.publication_title) & " - " & qGetNistInfo.pub_report_num;
				sRetVal.series = convertNistPub(qGetNistInfo.publication_title);
      }
      </cfscript>

      <cfcatch>
        <cflog text="RemoteGateway Error: getNistInfo: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
        <cfset sRetVal.citation = "">
      </cfcatch>

      </cftry>

      <cfreturn sRetVal>
  </cffunction>

  <!--- * * * * * * * * * * * * * * * * * * * * * 6 OTHER INFORMATION  * * * * * * * * * * * * * * * * * * * * * --->
  <cffunction name="getOtherInfo" access="remote" output="false" returnType="any">
    <cfargument name="pid" displayname="Publication ID" required="yes" type="numeric">

    <cfset sRetVal = structNew()>

    <cftry>

      <cfquery name="qGetOtherInfo" datasource="#variables.PublicationsDSN#">
        SELECT other_title
        FROM NIKE_ADMIN.OTHER_VW_CS
        WHERE pub_id = <cfqueryparam cfsqltype="cf_sql_integer" value="#arguments.pid#">
      </cfquery>

      <cfif qGetOtherInfo.recordcount>
        <cfset sRetVal.citation = qGetOtherInfo.other_title>
      <cfelse>
        <cfset sRetVal.citation = "">
      </cfif>

      <cfcatch>
        <cflog text="RemoteGateway Error: qGetOtherInfo: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
        <cfset sRetVal.citation = "">
      </cfcatch>

      </cftry>

      <cfreturn sRetVal>
  </cffunction>

  <!--- * * * * * * * * * * * * * * * * * * * * * 7 BOOK CHAPTER INFORMATION  * * * * * * * * * * * * * * * * * * * * * --->
  <cffunction name="getBookChapterInfo" access="remote" output="false" returnType="any">
    <cfargument name="pid" displayname="Publication ID" required="yes" type="numeric">

    <cfset sRetVal = structNew()>

    <cftry>

      <cfquery name="qGetBookInfo" datasource="#variables.PublicationsDSN#">
        SELECT book_title, publisher_name, publisher_address, publisher_city, publisher_state, publisher_country
        FROM NIKE_ADMIN.BOOK_CHAPTER_VW_CS
        WHERE pub_id = <cfqueryparam cfsqltype="cf_sql_integer" value="#arguments.pid#">
      </cfquery>

      <cfscript>
        if(qGetBookInfo.recordcount){
          if(len(trim(qGetBookInfo.publisher_city))){
    				variables.book_loc = qGetBookInfo.publisher_city & ", ";
    			}
    			if(qGetBookInfo.publisher_state EQ "1" ) {
    				variables.book_loc = variables.book_loc & qGetBookInfo.publisher_country;
    			} else {
    				variables.book_loc = variables.book_loc & qGetBookInfo.publisher_state;
    			}

    			sRetVal.citation = qGetBookInfo.book_title;
    			sRetVal.publisher_info = qGetBookInfo.publisher_name & ", " & variables.book_loc;
        } else {
          sRetVal.citation = "";
          sRetVal.publisher_info = "";
        }
      </cfscript>

      <cfcatch>
        <cflog text="RemoteGateway Error: qGetBookInfo: #cfcatch.Message# #cfcatch.Detail#" file="CFC_Errors" date="yes" time="yes" application="no">
        <cfset sRetVal.citation = "">
      </cfcatch>

      </cftry>

      <cfreturn sRetVal>
  </cffunction>


  <!--- *********** Package *********** --->
  <!--- *********** Private *********** --->
  <!--- convertOracleOutput --->
  <cffunction name="convertOracleOutput" returntype="any" access="public" output="false"
      displayname="Convert Oracle Output" hint="I return a string correctly formated for HTML display."
      description="I return a string correctly formated for HTML display.">

      <cfargument name="string" type="string" required="true" hint="I am the string to convert. I am required." />

      <cfset var entityArray = this.getEntityArray() />
      <cfset var newString = duplicate(arguments.string) />
      <!--- regex to trim spaces and set super and sub scripts
        ^uCONTENT^ is superscripted, ^dCONTENT^ is subscripted --->
      <cfscript>
        newString = trim(newString);//trim
        newString = rereplace(newString,"\s+"," ", "ALL");//delete multiple spaces
        newString = rereplacenocase(newString, "\^d(.*?)\^", "<sub>\1</sub>","ALL");//convert ^d--^ to subscripts
        newString = rereplacenocase(newString, "\^u(.*?)\^", "<sup>\1</sup>","ALL");//convert ^u--^ to superscripts
      </cfscript>

    <!--- now do the HTML entity replcement --->
      <cfloop array="#entityArray#" index="x">
        <cftry>
          <cfset newString = replace(newString, x.key, x.value, "all") />
          <cfcatch>
            <cflog text="convertOracleOutput: #cfcatch.Message# #cfcatch.Detail# Input string: (#arguments.string#) " type="Error" file="RemoteGateway_Errors" date="yes" time="yes" application="no">
          </cfcatch>
        </cftry>
      </cfloop>

      <cfreturn newString />
  </cffunction>

  <cfscript>
    function convertNistPub(t) {
      switch(t) {
        case "NISTJRES":
          npub = "Journal of Research (NIST JRES)";
          break;
        case "JPCRD":
          npub ="J. Phys. & Chem. Ref. Data (JPCRD)";
          break;
        case "NIST HB":
          npub = "Handbook (NIST HB)";
          break;
        case "NIST SP": case "NISTSP":
          npub = "Special Publication (NIST SP)";
          break;
        case "NIST TN": case "NISTTN":
          npub = "Technical Note (NIST TN)";
          break;
        case "NIST MN": case "NISTMN":
          npub = "Monograph (NIST MN)";
          break;
        case "NIST NSRDS":
          npub = "Nat'l Std. Ref. Data Series (NIST NSRDS)";
          break;
        case "NIST FIPS":
          npub = "Federal Inf. Process. Stds. (NIST FIPS)";
          break;
        case "NIST LP":
          npub = "List of Publications (NIST LP)";
          break;
        case "NIST IR": case "NISTIR":
          npub = "NIST Interagency/Internal Report (NISTIR)";
          break;
        case "LC":
          npub = "Letter Circular";
          break;
        case "BSS":
          npub = "Building Science Series";
          break;
        default:
          npub = t;
      }
      return npub;
    }

    function convertPubType(p) {
      switch(p) {
        case "0":
          pt = "Journals";
          break;
        case "1":
          pt ="Books";
          break;
        case "2":
          pt = "Encyclopedias";
          break;
        case "3":
          pt = "Conferences";
          break;
        case "4":
          pt = "Websites";
          break;
        case "5":
          pt = "NIST Pubs";
          break;
        case "6":
          pt = "Others";
          break;
        case "7":
          pt = "Book Chapters";
          break;
        case "8":
          pt = "Talks";
          break;
        default:
          pt = "";
      }
      return pt;
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

    function getEntityArray(){

      var entityMap = structNew();
      var entityArray = arrayNew(1);

      entityMap.lessThanOrEqualTo.key = "{less that or equal to}";
      entityMap.lessThanOrEqualTo.value =  "&##8804;";
      arrayAppend(entityArray, entityMap.lessThanOrEqualTo);

      entityMap.approxEquel.key = "{approximatelay equal}";
      entityMap.approxEquel.value =  "&##8773;";
      arrayAppend(entityArray, entityMap.approxEquel);

      entityMap.plusOrMinus.key = "{+ or -}";
      entityMap.plusOrMinus.value =  "&##177;";
      arrayAppend(entityArray, entityMap.plusOrMinus);


      entityMap.greaterThanOrEQTo.key = "{greater than or equal to}";
      entityMap.greaterThanOrEQTo.value =  "&##8805;";
      arrayAppend(entityArray, entityMap.greaterThanOrEQTo);

      entityMap.multiplyedBy.key = "{multiplied by}";
      entityMap.multiplyedBy.value =  "&##215;";
      arrayAppend(entityArray, entityMap.multiplyedBy);

      entityMap.multiply.key = "{multiply}";
      entityMap.multiply.value =  "&##215;";
      arrayAppend(entityArray, entityMap.multiply);

      entityMap.dividedBy.key = "{divided by}";
      entityMap.dividedBy.value =  "&##247;";
      arrayAppend(entityArray, entityMap.dividedBy);

      entityMap.infinity.key = "{infinity}";
      entityMap.infinity.value =  "&##8734;";
      arrayAppend(entityArray, entityMap.infinity);

      entityMap.proportionalTo.key = "{proportional to}";
      entityMap.proportionalTo.value =  "&##8733;";
      arrayAppend(entityArray, entityMap.proportionalTo);

      entityMap.notEqual.key = "{not equal}";
      entityMap.notEqual.value =  "&##8800;";
      arrayAppend(entityArray, entityMap.notEqual);

      entityMap.degree.key = "{degree}";
      entityMap.degree.value =  "&##176;";
      arrayAppend(entityArray, entityMap.degree);

      entityMap.oneQuarter.key = "{1/4}";
      entityMap.oneQuarter.value =  "1&##8260;4;";
      arrayAppend(entityArray, entityMap.oneQuarter);

      entityMap.oneHalf.key = "{1/2}";
      entityMap.oneHalf.value =  "1&##8260;2";
      arrayAppend(entityArray, entityMap.oneHalf);

      entityMap.threeQuarters.key = "{3/4}";
      entityMap.threeQuarters.value =  "3&##8260;4";
      arrayAppend(entityArray, entityMap.threeQuarters);

      entityMap.bullet.key = "{bullet}";
      entityMap.bullet.value =  "&##8226";
      arrayAppend(entityArray, entityMap.bullet);

      entityMap.longDash.key = "{long dash}";
      entityMap.longDash.value =  "&##8211;";
      arrayAppend(entityArray, entityMap.longDash);

      entityMap.emptySet.key = "{empty set}";
      entityMap.emptySet.value =  "&##8709;";
      arrayAppend(entityArray, entityMap.emptySet);

      entityMap.memberOf.key = "{member of}";
      entityMap.memberOf.value =  "&##8715;";
      arrayAppend(entityArray, entityMap.memberOf);

      entityMap.intergral.key = "{intergral}";
      entityMap.intergral.value =  "&##8747;";
      arrayAppend(entityArray, entityMap.intergral);

      entityMap.squareRoot.key = "{square root}";
      entityMap.squareRoot.value =  "&##8730;";
      arrayAppend(entityArray, entityMap.squareRoot);

      entityMap.mathPlus.key = "{math plus}";
      entityMap.mathPlus.value =  "+";
      arrayAppend(entityArray, entityMap.mathPlus);

      entityMap.mathMinus.key = "{math minus}";
      entityMap.mathMinus.value =  "-";
      arrayAppend(entityArray, entityMap.mathMinus);

      entityMap.mathEqual.key = "{math equal}";
      entityMap.mathEqual.value =  "=";
      arrayAppend(entityArray, entityMap.mathEqual);

      entityMap.mathstar.key = "{math star}";
      entityMap.mathstar.value =  "*";
      arrayAppend(entityArray, entityMap.mathstar);

      entityMap.copyRight.key = "{copyright}";
      entityMap.copyRight.value =  "&##169;";
      arrayAppend(entityArray, entityMap.copyRight);

      entityMap.alpha.key = "{alpha}";
      entityMap.alpha.value =  "&##945;";
      arrayAppend(entityArray, entityMap.alpha);
      entityMap.CapAlpha.key = "{Alpha}";
      entityMap.CapAlpha.value =  "&##913;";
      arrayAppend(entityArray, entityMap.Capalpha);

      entityMap.beta.key = "{beta}";
      entityMap.beta.value =  "&##946;";
      arrayAppend(entityArray, entityMap.beta);
      entityMap.capBeta.key = "{Beta}";
      entityMap.capBeta.value =  "&##914;";
      arrayAppend(entityArray, entityMap.capBeta);

      entityMap.gamma.key = "{gamma}";
      entityMap.gamma.value =  "&##947;";
      arrayAppend(entityArray, entityMap.gamma);
      entityMap.capGamma.key = "{Gamma}";
      entityMap.capGamma.value =  "&##915;";
      arrayAppend(entityArray, entityMap.capGamma);

      entityMap.delta.key = "{delta}";
      entityMap.delta.value =  "&##948;";
      arrayAppend(entityArray, entityMap.delta);
      entityMap.capdelta.key = "Delta";
      entityMap.capdelta.value =  "&##916;";
      arrayAppend(entityArray, entityMap.capdelta);

      entityMap.epsilon.key = "{epsilon}";
      entityMap.epsilon.value =  "&##949;";
      arrayAppend(entityArray, entityMap.epsilon);
      entityMap.capepsilon.key = "{Epsilon}";
      entityMap.capepsilon.value =  "&##917;";
      arrayAppend(entityArray, entityMap.capepsilon);

      entityMap.Zeta.key = "{zeta}";
      entityMap.Zeta.value =  "&##950;";
      arrayAppend(entityArray, entityMap.Zeta);
      entityMap.capZeta.key = "{Zeta}";
      entityMap.capZeta.value =  "&##918;";
      arrayAppend(entityArray, entityMap.capZeta);

      entityMap.Iota.key = "{iota}";
      entityMap.Iota.value =  "&##953;";
      arrayAppend(entityArray, entityMap.Iota);
      entityMap.capIota.key = "{Iota}";
      entityMap.capIota.value =  "&##921;";
      arrayAppend(entityArray, entityMap.capIota);

      entityMap.Kappa.key = "{kappa}";
      entityMap.Kappa.value =  "&##954;";
      arrayAppend(entityArray, entityMap.Kappa);
      entityMap.capKappa.key = "Kappa";
      entityMap.capKappa.value =  "&##922;";
      arrayAppend(entityArray, entityMap.capKappa);

      entityMap.Lambda.key = "{lambda}";
      entityMap.Lambda.value =  "&##955;";
      arrayAppend(entityArray, entityMap.Lambda);
      entityMap.capLambda.key = "{Lambda}";
      entityMap.capLambda.value =  "&##923;";
      arrayAppend(entityArray, entityMap.capLambda);

      entityMap.Omicron.key = "{omicron}";
      entityMap.Omicron.value =  "&##959;";
      arrayAppend(entityArray, entityMap.Omicron);
      entityMap.capOmicron.key = "{Omicron}";
      entityMap.capOmicron.value =  "&##927;";
      arrayAppend(entityArray, entityMap.capOmicron);

      entityMap.Rho.key = "{rho}";
      entityMap.Rho.value =  "&##961;";
      arrayAppend(entityArray, entityMap.Rho);
      entityMap.capRho.key = "{Rho}";
      entityMap.capRho.value =  "&##929;";
      arrayAppend(entityArray, entityMap.capRho);

      entityMap.Sigma.key = "{sigma}";
      entityMap.Sigma.value =  "&##962;";
      arrayAppend(entityArray, entityMap.Sigma);
      entityMap.capSigma.key = "{Sigma}";
      entityMap.capSigma.value =  "&##931;";
      arrayAppend(entityArray, entityMap.capSigma);

      entityMap.Tau.key = "{tau}";
      entityMap.Tau.value =  "&##964;";
      arrayAppend(entityArray, entityMap.Tau);
      entityMap.capTau.key = "{Tau}";
      entityMap.capTau.value =  "&##932;";
      arrayAppend(entityArray, entityMap.capTau);

      entityMap.Upsilon.key = "{upsilon}";
      entityMap.Upsilon.value =  "&##965;";
      arrayAppend(entityArray, entityMap.Upsilon);
      entityMap.capUpsilon.key = "Upsilon";
      entityMap.capUpsilon.value =  "&##936;";
      arrayAppend(entityArray, entityMap.capUpsilon);

      entityMap.Omega.key = "{omega}";
      entityMap.Omega.value =  "&##969;";
      arrayAppend(entityArray, entityMap.Omega);
      entityMap.capOmega.key = "Omega";
      entityMap.capOmega.value =  "&##937;";
      arrayAppend(entityArray, entityMap.capOmega);

      entityMap.Umlat.key = "{umlat}";
      entityMap.Umlat.value =  "&##252;";
      arrayAppend(entityArray, entityMap.Umlat);
      entityMap.capUmlat.key = "{Umlat}";
      entityMap.capUmlat.value =  "&##220;";
      arrayAppend(entityArray, entityMap.capUmlat);

      entityMap.Omlat.key = "{omlat}";
      entityMap.Omlat.value =  "&##246;";
      arrayAppend(entityArray, entityMap.Omlat);
      entityMap.capOmlat.key = "{Omlat}";
      entityMap.capOmlat.value =  "&##214;";
      arrayAppend(entityArray, entityMap.capOmlat);

      entityMap.emlat.key = "{emlat}";
      entityMap.emlat.value =  "&##235;";
      arrayAppend(entityArray, entityMap.emlat);

      entityMap.Pi.key = "{pi}";
      entityMap.Pi.value =  "&##960;";
      arrayAppend(entityArray, entityMap.Pi);
      entityMap.capPi.key = "{Pi}";
      entityMap.capPi.value =  "&##928;";
      arrayAppend(entityArray, entityMap.capPi);

      entityMap.Phi.key = "{phi}";
      entityMap.Phi.value =  "&##966;";
      arrayAppend(entityArray, entityMap.Phi);
      entityMap.capPhi.key = "{Phi}";
      entityMap.capPhi.value =  "&##934;";
      arrayAppend(entityArray, entityMap.capPhi);

      entityMap.Psi.key = "{psi}";
      entityMap.Psi.value =  "&##968;";
      arrayAppend(entityArray, entityMap.Psi);
      entityMap.capPsi.key = "{Psi}";
      entityMap.capPsi.value =  "&##936;";
      arrayAppend(entityArray, entityMap.capPsi);

      entityMap.Chi.key = "{chi}";
      entityMap.Chi.value =  "&##967;";
      arrayAppend(entityArray, entityMap.Chi);
      entityMap.capChi.key = "{Chi}";
      entityMap.capChi.value =  "&##935;";
      arrayAppend(entityArray, entityMap.capChi);

      entityMap.Theta.key = "{theta}";
      entityMap.Theta.value =  "&##952;";
      arrayAppend(entityArray, entityMap.Theta);
      entityMap.capTheta.key = "{Theta}";
      entityMap.capTheta.value =  "&##952;";
      arrayAppend(entityArray, entityMap.capTheta);

      entityMap.eta.key = "{eta}";
      entityMap.eta.value =  "&##919;";
      arrayAppend(entityArray, entityMap.eta);
      entityMap.capeta.key = "{Eta}";
      entityMap.capeta.value =  "&##951;";
      arrayAppend(entityArray, entityMap.capeta);

      entityMap.Mu.key = "{mu}";
      entityMap.Mu.value =  "&##965;";
      arrayAppend(entityArray, entityMap.Mu);
      entityMap.capMu.key = "{Mu}";
      entityMap.capMu.value =  "&##324;";
      arrayAppend(entityArray, entityMap.capMu);

      entityMap.Nu.key = "{nu}";
      entityMap.Nu.value =  "&##325;";
      arrayAppend(entityArray, entityMap.Nu);
      entityMap.capNu.key = "{Nu}";
      entityMap.capNu.value =  "&##957;";
      arrayAppend(entityArray, entityMap.capNu);

      entityMap.Xi.key = "{xi}";
      entityMap.Xi.value =  "&##958;";
      arrayAppend(entityArray, entityMap.Xi);
      entityMap.capXi.key = "{Xi}";
      entityMap.capXi.value =  "&##926;";
      arrayAppend(entityArray, entityMap.capXi);

      entityMap.bodlEnd.key = "{/B}";
      entityMap.bodlEnd.value =  "";
      arrayAppend(entityArray, entityMap.bodlEnd);
      entityMap.boldStart.key = "{B}";
      entityMap.boldStart.value =  "";
      arrayAppend(entityArray, entityMap.boldStart);

      entityMap.italiacEnd.key = "{/I}";
      entityMap.italiacEnd.value =  "";
      arrayAppend(entityArray, entityMap.italiacEnd);
      entityMap.italicStart.key = "{I}";
      entityMap.italicStart.value =  "";
      arrayAppend(entityArray, entityMap.italicStart);

      return entityArray;
    }


    /**
    * Converts an array of structures to a CF Query Object.
    * 6-19-02: Minor revision by Rob Brooks-Bilson (rbils@amkor.com)
    *
    * Update to handle empty array passed in. Mod by Nathan Dintenfass. Also no longer using list func.
    *
    * @param Array      The array of structures to be converted to a query object. Assumes each array element contains structure with same (Required)
    * @return Returns a query object.
    * @author David Crawford (rbils@amkor.comdcrawford@acteksoft.com)
    * @version 2, March 19, 2003
    */
    function arrayOfStructuresToQuery(theArray){
        var colNames = "";
        var theQuery = queryNew("");
        var i=0;
        var j=0;
        //if there's nothing in the array, return the empty query
        if(NOT arrayLen(theArray))
         return theQuery;
        //get the column names into an array =
        colNames = structKeyArray(theArray[1]);
        //build the query based on the colNames
        theQuery = queryNew(arrayToList(colNames));
        //add the right number of rows to the query
        queryAddRow(theQuery, arrayLen(theArray));
        //for each element in the array, loop through the columns, populating the query
        for(i=1; i LTE arrayLen(theArray); i=i+1){
         for(j=1; j LTE arrayLen(colNames); j=j+1){
          querySetCell(theQuery, colNames[j], theArray[i][colNames[j]], i);
         }
        }
        return theQuery;
    }

    /**
    * Removes duplicate rows from a query based on a key column.
    * Modded by Ray Camden to remove evaluate
    *
    * @param theQuery      The query to dedupe. (Required)
    * @param keyColumn      Column name to check for duplicates. (Required)
    * @return Returns a query.
    * @author Matthew Fusfield (matt@fus.net)
    * @version 1, December 19, 2008
    */
    function QueryDeDupe(theQuery,keyColumn) {
        var checkList='';
        var newResult=QueryNew(theQuery.ColumnList);
        var keyvalue='';
        var q = 1;
        var x = "";

        // loop through each row of the source query
        for (;q LTE theQuery.RecordCount;q=q+1) {

         keyvalue = theQuery[keycolumn][q];
         // see if the primary key value has already been used
         if (NOT ListFind(checkList,keyvalue)) {

          /* this is not a duplicate, so add it to the list and copy
           the row to the destination query */
          checkList=ListAppend(checklist,keyvalue);
          QueryAddRow(NewResult);

          // copy all columns from source to destination for this row
          for (x=1;x LTE ListLen(theQuery.ColumnList);x=x+1) {
           QuerySetCell(NewResult,ListGetAt(theQuery.ColumnList,x),theQuery[ListGetAt(theQuery.ColumnList,x)][q]);
          }
         }
        }
        return NewResult;
    }

    /**
    * Case-insensitive function for removing duplicate entries in a list.
    * Based on dedupe by Raymond Camden
    *
    * @param list      List to be modified. (Required)
    * @return Returns a list.
    * @author Jeff Howden (cflib@jeffhowden.com)
    * @version 1, July 2, 2008
    */
    function ListDeleteDuplicatesNoCase(list) {
      var i = 1;
      var delimiter = ',';
      var returnValue = '';
      if(ArrayLen(arguments) GTE 2)
        delimiter = arguments[2];
      list = ListToArray(list, delimiter);
      for(i = 1; i LTE ArrayLen(list); i = i + 1)
        if(NOT ListFindNoCase(returnValue, list[i], delimiter))
          returnValue = ListAppend(returnValue, list[i], delimiter);
      return returnValue;
    }
  </cfscript>

</cfcomponent>
import { Injectable } from '@angular/core';
import { HttpClient, HttpParams } from '@angular/common/http';

import * as _ from 'lodash';

import { BaseService } from '../dstore/services/base.service';
import { map } from 'rxjs/operators';

@Injectable()
export class CommentService {
  private server: string;
  constructor(private http: HttpClient) {
    this.server = BaseService.serverHosts.operationServer;
  }

  list(appName: string, query?: { [key: string]: string }) {
    const params = Object.entries(query)
      .map(a => `${a[0]}=${encodeURIComponent(a[1])}`)
      .join('&');
    return this.http.get(`${this.server}/api/comment/app/${appName}?` + params).pipe(
      map((resp: { comments: Comment[] }) => {
        resp.comments.map(comment => (comment.rate /= 2));
        const hot = resp.comments
          .filter(c => c.likeCount > 0)
          .sort((c1, c2) => c1.likeCount - c2.likeCount)
          .slice(0, 3)
          .map(c => ((c.hot = true), c));
        resp.comments = resp.comments
          .filter(c => !hot.includes(c))
          .sort((c1, c2) => c2.createTime.localeCompare(c1.createTime));
        return [...hot, ...resp.comments];
      }),
    );
  }

  create(appName: string, content: string, rate: number, version: string) {
    const c: Comment = {
      appName,
      content,
      rate,
      version,
    };
    return this.http.post(`${this.server}/api/comment/app/${appName}`, c);
  }

  own(appName: string, version: string) {
    return this.http
      .get<{ comment: Comment }>(
        `${this.server}/api/comment/app/${appName}/own?version=${encodeURIComponent(version)}`,
      )
      .pipe(
        map(resp => {
          if (!resp.comment) {
            return undefined;
          }
          resp.comment.rate /= 2;
          return resp.comment;
        }),
      );
  }
  like(cid: number) {
    return this.http.post(`${this.server}/api/comment/like/${cid}`, null);
  }
  dislike(cid: number) {
    return this.http.post(`${this.server}/api/comment/dislike/${cid}`, null);
  }
}

export interface Comment {
  id?: number;
  appName: string;
  createTime?: string;
  content: string;
  rate: number;
  version: string;
  userID?: number;
  likeCount?: number;
  likeByMe?: boolean;
  hot?: boolean;
}

interface User {
  id: number;
  name: string;
}
